#include "MatTestPass.hpp"

#include <ranges>
#include <utility>
#include <Engine.Accel.Command/CommandBuffer.hpp>
#include <Engine.Accel.Compile/VkEffectCompiler.hpp>
#include <Engine.Accel.Compile/Profile/EffectProfile.hpp>
#include <Engine.Accel.Compile/Spec/SimpleEffectSpecification.hpp>
#include <Engine.Accel.Pass/VkAccelerationPassFactory.hpp>
#include <Engine.Accel.Pipeline/GraphicsPipeline.hpp>
#include <Engine.Assets/Types/Texture/TextureAsset.hpp>
#include <Engine.Common/Math/Coordinates.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Driver.Vulkan/VkRCmdTranslator.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/BrdfIrradiance.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/BrdfLut.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/BrdfPrefilter.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/DirectionalShadow.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneDepth.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneView.hpp>
#include <Engine.GFX.RenderGraph/Symbol/ScopedSymbolContext.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>
#include <Engine.GFX/Buffer/Buffer.hpp>
#include <Engine.GFX/Cache/GlobalCacheCtrl.hpp>
#include <Engine.GFX/Pool/SceneResourcePool.hpp>
#include <Engine.GFX/Texture/SparseTexturePage.hpp>
#include <Engine.Gfx/Texture/SparseTextureView.hpp>
#include <Engine.GFX/Texture/TextureFactory.hpp>
#include <Engine.Gfx/Texture/TextureView.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBuffer.hpp>
#include <Engine.GFX.Scene.Model/StaticGeometryModel.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Reflect/ExactType.hpp>
#include <Engine.Render.Scene/RenderSceneSystem.hpp>
#include <Heliogrim/SceneComponent.hpp>

using namespace hg::engine::render;
using namespace hg::engine::accel;
using namespace hg::engine::gfx;
using namespace hg;

[[nodiscard]] static smr<const GraphicsPass> build_test_pass();

[[nodiscard]] static EffectCompileResult build_test_pipeline(
	mref<smr<const AccelerationEffect>> effect_,
	mref<smr<const GraphicsPass>> pass_
);

/**/

constexpr static Array possibleSampler = {
	lang::SymbolId::from("brdf-lut-sampler"sv),
	lang::SymbolId::from("brdf-pref-sampler"sv),
	lang::SymbolId::from("brdf-irrad-sampler"sv),
	lang::SymbolId::from("shadow-dir-sampler"sv),
	lang::SymbolId::from("mat-static-0-sampler"sv),
	lang::SymbolId::from("mat-static-1-sampler"sv),
	lang::SymbolId::from("mat-static-2-sampler"sv)
};

/**/

void MatTestPass::destroy() noexcept {
	MeshSubPass::destroy();

	auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();
	device->vkDevice().destroySemaphore(std::exchange(_tmpSignal, nullptr));

	_cameraBufferView.reset();
	if (_cameraBuffer.memory && _cameraBuffer.memory->mapping) {
		_cameraBuffer.unmap();
	}
	_cameraBuffer.destroy();

	_sceneShadowView.reset();
	_sceneLightInfoView.reset();
	_sceneLightView.reset();
	_staticInstanceView.reset();

	_shadowDirView.reset();

	if (_depthSampler) {
		_depthSampler->destroy();
		_depthSampler.reset();
	}

	if (_sampler) {
		_sampler->destroy();
		_sampler.reset();
	}

	if (_framebuffer) {
		_framebuffer->destroy();
		_framebuffer.reset();
	}

	for (auto&& entry : _batches) {
		entry.second.compiled.pipeline.reset();
	}
	_batches.clear();
	_pass.reset();
}

void MatTestPass::declareTransforms(ref<graph::ScopedSymbolContext> symCtx_) noexcept {
	MeshSubPass::declareTransforms(symCtx_);

	symCtx_.registerImportSymbol(makeSceneColorSymbol(), &_resources.inOutSceneColor);
	symCtx_.registerExportSymbol(makeSceneColorSymbol(), &_resources.inOutSceneColor);

	symCtx_.registerImportSymbol(makeSceneDepthSymbol(), &_resources.inOutDepthBuffer);
	symCtx_.registerExportSymbol(makeSceneDepthSymbol(), &_resources.inOutDepthBuffer);
}

void MatTestPass::iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

	if (_pass.empty()) {
		_pass = build_test_pass();
	}

	/**/

	for (auto& batch : std::ranges::views::values(_batches)) {
		batch.materials.clear();
		batch.models.clear();
	}

	/**/

	auto sceneViewRes = symCtx_.getImportSymbol(makeSceneViewSymbol());
	auto sceneView = sceneViewRes->load<smr<const gfx::scene::SceneView>>();

	size_t idx = 0;

	const auto sys = sceneView->getRenderSceneSystem();
	sys->getRegistry().forEach<gfx::scene::StaticGeometryModel>(
		[this, &idx](const gfx::scene::StaticGeometryModel& model_) {

			auto material = model_.material(0uL);
			auto guard = material->acquire(resource::ResourceUsageFlag::eRead);

			auto proto = guard->getPrototype();
			assert(proto->getMaterialEffects().size() == 1uLL);

			auto batchIt = _batches.find(proto->getMaterialEffects().front().effect);
			if (batchIt == _batches.end()) {

				const auto result = _batches.emplace(
					proto->getMaterialEffects().front().effect,
					Payload {
						.effect = proto->getMaterialEffects().front(),
						.compiled = {},
						.materials = {},
						.models = {}
					}
				);
				assert(result.second);

				batchIt = result.first;

			}

			auto& payload = batchIt->second;

			payload.materials.emplace_back(material);
			payload.models.emplace_back(std::addressof(model_));

			/**/

			++idx;
		}
	);
}

void MatTestPass::resolve() noexcept {

	for (auto& batch : std::ranges::views::values(_batches)) {
		if (batch.compiled.flag == EffectCompileResultFlag::eUnknown) {
			batch.compiled = build_test_pipeline(clone(batch.effect.effect), clone(_pass));
		}
	}
}

void MatTestPass::execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

	if (_batches.empty()) {
		return;
	}

	/**/

	auto sceneColorRes = symCtx_.getExportSymbol(makeSceneColorSymbol());
	auto sceneDepthRes = symCtx_.getImportSymbol(makeSceneDepthSymbol());

	const auto& sceneColorTex = sceneColorRes->load<smr<const TextureLikeObject>>();
	const auto& sceneDepthTex = sceneDepthRes->load<smr<const TextureLikeObject>>();

	/**/

	if (
		not _framebuffer.empty() &&
		(
			_framebuffer->attachments().front() != sceneColorTex ||
			static_cast<ptr<void>>(_framebuffer->attachments().back().get()) != sceneDepthRes.get()
		)
	) {
		_framebuffer->device()->vkDevice().destroySemaphore(_tmpSignal);
		_framebuffer->destroy();
		_framebuffer.reset();
	}

	if (_framebuffer.empty()) {

		const Vector outputSymbols { makeSceneColorSymbol(), makeSceneDepthSymbol() };
		_framebuffer = make_smr<Framebuffer>(Engine::getEngine()->getGraphics()->getCurrentDevice());

		for (const auto& output : outputSymbols) {

			auto data = symCtx_.getExportSymbol(clone(output));

			const auto& texture = data->load<smr<TextureLikeObject>>();
			_framebuffer->addAttachment(clone(texture));
		}

		{
			const auto texture = Cast<Texture>(sceneColorTex.get());
			_framebuffer->setExtent(texture->extent());
		}

		_framebuffer->setRenderPass(clone(_pass));
		_framebuffer->setup();

		_tmpSignal = _framebuffer->device()->vkDevice().createSemaphore({});
	}

	/**/

	if (_sampler == nullptr) {
		_sampler = make_uptr<TextureSampler>();
		_sampler->setup(_framebuffer->device());
	}

	if (_depthSampler == nullptr) {
		_depthSampler = make_uptr<TextureSampler>();
		_depthSampler->magnification() = vk::Filter::eLinear;
		_depthSampler->minification() = vk::Filter::eLinear;
		_depthSampler->borderColor() = vk::BorderColor::eFloatOpaqueWhite;
		_depthSampler->addressModeU() = vk::SamplerAddressMode::eClampToEdge;
		_depthSampler->addressModeV() = vk::SamplerAddressMode::eClampToEdge;
		_depthSampler->addressModeW() = vk::SamplerAddressMode::eClampToEdge;
		_depthSampler->setup(_framebuffer->device());
	}

	/**/

	if (_cameraBuffer.buffer == nullptr) {

		const auto& device = _framebuffer->device();

		vk::BufferCreateInfo bci {
			vk::BufferCreateFlags {}, sizeof(math::mat4) * 3, vk::BufferUsageFlagBits::eUniformBuffer,
			vk::SharingMode::eExclusive, 0uL, nullptr
		};
		_cameraBuffer.buffer = device->vkDevice().createBuffer(bci);
		_cameraBuffer.device = device->vkDevice();

		const auto allocResult = memory::allocate(
			device->allocator(),
			device,
			_cameraBuffer.buffer,
			MemoryProperties { MemoryProperty::eHostVisible },
			_cameraBuffer.memory
		);

		assert(_cameraBuffer.buffer);
		assert(_cameraBuffer.memory);

		_cameraBuffer.size = _cameraBuffer.memory->size;
		_cameraBuffer.usageFlags = vk::BufferUsageFlagBits::eUniformBuffer;

		_cameraBuffer.bind();
	}

	auto sceneViewRes = symCtx_.getImportSymbol(makeSceneViewSymbol());
	auto sceneView = sceneViewRes->load<smr<gfx::scene::SceneView>>();

	struct TmpStruct {
		math::mat4 mvp;
		math::mat4 proj;
		math::mat4 view;
	};

	auto tmp = TmpStruct {
		sceneView->getProjectionMatrix() * sceneView->getViewMatrix(),
		sceneView->getProjectionMatrix(),
		sceneView->getViewMatrix()
	};

	_cameraBuffer.write<TmpStruct>(&tmp, 1uL);

	if (_cameraBufferView == nullptr) {
		_cameraBufferView = make_uptr<UniformBufferView>();
		_cameraBufferView->storeBuffer(&_cameraBuffer);
	}

	/**/

	if (_staticInstanceView == nullptr) {
		_staticInstanceView = make_uptr<gfx::StorageBufferView>();
	}

	const auto sceneSystem = sceneView->getRenderSceneSystem();
	const auto scenePool = sceneSystem->getSceneResourcePool();

	const auto& staticInstancePool = scenePool->staticInstancePool;
	_staticInstanceView->storeBuffer(staticInstancePool.getPoolView());

	/**/

	if (_sceneLightInfoView == nullptr) {
		_sceneLightInfoView = make_uptr<gfx::UniformBufferView>();
	}

	if (_sceneLightView == nullptr) {
		_sceneLightView = make_uptr<StorageBufferView>();
	}

	const auto& sceneLightInfoBuffer = scenePool->sceneLightInfoBuffer;
	const auto& sceneLightPool = scenePool->lightSourcePool;

	_sceneLightInfoView->storeBuffer(&sceneLightInfoBuffer);
	_sceneLightView->storeBuffer(sceneLightPool.getPoolView());

	/**/

	if (_sceneShadowView == nullptr) {
		_sceneShadowView = make_uptr<StorageBufferView>();
	}

	const auto& sceneShadowPool = scenePool->shadowSourcePool;
	_sceneShadowView->storeBuffer(sceneShadowPool.getPoolView());

	/**/

	cmd::RenderCommandBuffer cmd {};
	cmd.begin();
	cmd.beginAccelPass({ _pass.get(), _framebuffer.get() });
	cmd.beginSubPass({});

	for (const auto& batch : std::ranges::views::values(_batches)) {

		const auto& compiled = batch.compiled;
		const auto& effect = batch.effect;

		cmd.bindGraphicsPipeline(clone(compiled.pipeline).into<GraphicsPipeline>());

		const auto* const gpipe = static_cast<const ptr<const GraphicsPipeline>>(compiled.pipeline.get());
		const auto& alias = compiled.alias;

		for (size_t idx = 0; idx < batch.models.size(); ++idx) {

			auto& model = batch.models[idx];
			const auto& modelResource = model->geometryResource();
			const auto& materialResource = batch.materials[idx];

			auto modelGuard = modelResource->acquire(resource::ResourceUsageFlag::eRead);
			auto materialGuard = materialResource->acquire(resource::ResourceUsageFlag::eRead);

			const auto& indexBuffer = modelGuard->indices();
			const auto& vertexBuffer = modelGuard->vertices();

			cmd.lambda(
				[&indexBuffer, &vertexBuffer](ref<AccelCommandBuffer> cmd_) {
					cmd_.bindIndexBuffer(indexBuffer.get());
					cmd_.bindVertexBuffer(0uL, vertexBuffer.get());
				}
			);

			for (const auto& group : gpipe->getBindingLayout().groups) {
				for (const auto& element : group.elements) {

					const auto symbolId = element.symbol->symbolId;
					const auto aliasId = alias.aliasOrValue(symbolId);

					const auto iter = std::ranges::find(
						effect.pattern->st.pairs,
						aliasId,
						[](const auto& pair_) {
							return pair_.second;
						}
					);

					if (iter != effect.pattern->st.pairs.end()) {

						const auto texture = materialGuard->getParam<smr<TextureResource>>(iter->first);
						auto textureGuard = texture->acquire(resource::ResourceUsageFlag::eRead);
						const auto* const assetName = Cast<assets::TextureAsset>(texture->getAssociation());

						if (auto* textureView = Cast<TextureView>(textureGuard.imm()->get())) {
							cmd.bindTexture(clone(symbolId), textureView);

						} else if (auto* sparseTextureView = Cast<SparseTextureView>(textureGuard.imm()->get())) {

							if (sparseTextureView->vkImageView() == nullptr) {
								TextureFactory::get()->buildView(*sparseTextureView, {});
							}

							/**/
							if (sparseTextureView->pages().front()->memory()->state() !=
								VirtualMemoryPageState::eLoaded) {

								textureGuard.release();

								auto* cc = Engine::getEngine()->getGraphics()->cacheCtrl();
								for (
									u32 mipLevel = sparseTextureView->minMipLevel();
									mipLevel < sparseTextureView->maxMipLevel();
									++mipLevel
								) {
									cc->markLoadedAsUsed(
										clone(texture),
										cache::TextureSubResource {
											.layer = 0,
											.mip = mipLevel,
											.offset = math::uivec3 {},
											.extent = math::uivec3 {
												sparseTextureView->width(), sparseTextureView->height(),
												sparseTextureView->depth()
											}
										}
									);
								}
							}
							/**/

							cmd.bindTexture(clone(symbolId), sparseTextureView);

						} else {
							assert(false);
						}

						continue;
					}

					constexpr auto viewSym = lang::SymbolId::from("view"_cs);
					if (aliasId == viewSym) {
						cmd.bindUniform(clone(symbolId), _cameraBufferView.get());
						continue;
					}

					constexpr auto modelSym = lang::SymbolId::from("model"_cs);
					if (aliasId == modelSym) {
						cmd.bindStorage(clone(symbolId), _staticInstanceView.get());
						continue;
					}

					constexpr auto lightInfoSym = lang::SymbolId::from("light-info"_cs);
					if (aliasId == lightInfoSym) {
						cmd.bindUniform(clone(symbolId), _sceneLightInfoView.get());
						continue;
					}

					constexpr auto lightsSym = lang::SymbolId::from("lights"_cs);
					if (aliasId == lightsSym) {
						cmd.bindStorage(clone(symbolId), _sceneLightView.get());
						continue;
					}

					// TODO: Take care of brdf helper objects for test case: `brdf-lut`, `brdf-prefilter`, `brdf-irradiance`
					constexpr auto brdfLutSym = lang::SymbolId::from("brdf-lut"_cs);
					if (aliasId == brdfLutSym) {

						const auto res = symCtx_.getImportSymbol(makeBrdfLutSymbol());
						const auto lutRes = res->load<smr<TextureLikeObject>>();
						auto* const lutTex = Cast<Texture>(lutRes.get());

						if (_brdfLutView == nullptr || _brdfLutView->owner() != lutTex) {
							_brdfLutView = lutTex->makeView(
								{ 0uL, lutTex->layer() },
								{ lutTex->width(), lutTex->height(), lutTex->depth() },
								{ 0uL, lutTex->mipLevels() }
							);
						}

						cmd.bindTexture(clone(symbolId), _brdfLutView.get());
						continue;
					}

					constexpr auto brdfPrefSym = lang::SymbolId::from("brdf-pref"_cs);
					if (aliasId == brdfPrefSym) {

						const auto res = symCtx_.getImportSymbol(makeBrdfPrefilterSymbol());
						const auto prefRes = res->load<smr<TextureLikeObject>>();
						auto* const prefTex = Cast<Texture>(prefRes.get());

						if (_brdfPrefView == nullptr || _brdfPrefView->owner() != prefTex) {
							_brdfPrefView = prefTex->makeView(
								{ 0uL, prefTex->layer() },
								{ prefTex->width(), prefTex->height(), prefTex->depth() },
								{ 0uL, prefTex->mipLevels() }
							);
						}

						cmd.bindTexture(clone(symbolId), _brdfPrefView.get());
						continue;
					}

					constexpr auto brdfIrradSym = lang::SymbolId::from("brdf-irrad"_cs);
					if (aliasId == brdfIrradSym) {

						const auto res = symCtx_.getImportSymbol(makeBrdfIrradianceSymbol());
						const auto irradRes = res->load<smr<TextureLikeObject>>();
						auto* const irradTex = Cast<Texture>(irradRes.get());

						if (_brdfIrradView == nullptr || _brdfIrradView->owner() != irradTex) {
							_brdfIrradView = irradTex->makeView(
								{ 0uL, irradTex->layer() },
								{ irradTex->width(), irradTex->height(), irradTex->depth() },
								{ 0uL, irradTex->mipLevels() }
							);
						}

						cmd.bindTexture(clone(symbolId), _brdfIrradView.get());
						continue;
					}

					constexpr auto shadowSym = lang::SymbolId::from("shadows"_cs);
					if (aliasId == shadowSym) {
						cmd.bindStorage(clone(symbolId), _sceneShadowView.get());
						continue;
					}

					constexpr auto shadowDirSym = lang::SymbolId::from("shadow-dir"_cs);
					if (aliasId == shadowDirSym) {

						const auto res = symCtx_.getImportSymbol(makeDirectionalShadowSymbol());
						const auto shadowRes = res->load<smr<TextureLikeObject>>();
						auto* const shadowTex = Cast<Texture>(shadowRes.get());

						if (_shadowDirView == nullptr || _shadowDirView->owner() != shadowTex) {
							_shadowDirView = shadowTex->makeView(
								{ 0uL, shadowTex->layer() },
								{ shadowTex->width(), shadowTex->height(), shadowTex->depth() },
								{ 0uL, shadowTex->mipLevels() }
							);
						}

						cmd.bindTexture(clone(symbolId), _shadowDirView.get());
						continue;
					}

					/**/

					if (std::ranges::any_of(
						possibleSampler,
						[&aliasId](const auto& sym_) {
							return aliasId == sym_;
						}
					)) {
						cmd.bindTextureSampler(clone(symbolId), _sampler.get());
						continue;
					}

					std::unreachable();
				}
			}

			cmd.drawStaticMeshIdx(1uL, model->_sceneInstanceIndex, indexBuffer->size() / sizeof(u32) / 3uLL, 0uL);
		}

	}

	cmd.endSubPass();
	cmd.endAccelPass();
	cmd.end();

	/**/

	auto translator = make_uptr<driver::vk::VkRCmdTranslator>();
	auto nativeBatch = (*translator)(&cmd);
	auto* const batch = static_cast<ptr<driver::vk::VkNativeBatch>>(nativeBatch.get());

	{
		{
			const auto res = symCtx_.getImportSymbol(makeDirectionalShadowSymbol());

			batch->_tmpWaits.insert_range(
				batch->_tmpWaits.end(),
				reinterpret_cast<Vector<VkSemaphore>&>(res->barriers)
			);
			for (auto i = batch->_tmpWaitFlags.size(); i < batch->_tmpWaits.size(); ++i) {
				batch->_tmpWaitFlags.push_back(VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
			}
			res->barriers.clear();
		}

		/**/

		batch->_tmpWaits.insert_range(
			batch->_tmpWaits.end(),
			reinterpret_cast<Vector<VkSemaphore>&>(sceneColorRes->barriers)
		);
		for (auto i = batch->_tmpWaitFlags.size(); i < batch->_tmpWaits.size(); ++i) {
			batch->_tmpWaitFlags.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		}
		batch->_tmpSignals.push_back(_tmpSignal);

		sceneColorRes->barriers.clear();
		sceneColorRes->barriers.push_back(_tmpSignal.operator VkSemaphore());
	}

	batch->commitAndDispose();
	nativeBatch.reset();
}

/**/

smr<const GraphicsPass> build_test_pass() {

	constexpr auto passFactory = VkAccelerationPassFactory {};
	auto graphicsPass = passFactory.buildGraphicsPass(
		{ makeSceneColorSymbol(), /**/makeSceneDepthSymbol() },
		{ makeSceneColorSymbol(), makeSceneDepthSymbol() }
	).value();

	return graphicsPass;
}

EffectCompileResult build_test_pipeline(
	mref<smr<const AccelerationEffect>> effect_,
	mref<smr<const GraphicsPass>> pass_
) {

	auto profile = make_smr<EffectProfile>(
		EffectProfile {
			._name = "Test-Mat-Profile",
			._definitions = {}
		}
	);

	/**/

	auto spec = make_smr<SimpleEffectSpecification>();
	spec->setPassSpec(
		make_uptr<GraphicsPassSpecification>(
			GraphicsPassSpecification {
				.depthCompareOp = DepthCompareOp::eLessEqual,
				.stencilCompareOp = StencilCompareOp::eNever,
				.stencilFailOp = StencilOp::eKeep,
				.stencilPassOp = StencilOp::eKeep,
				.stencilDepthFailOp = StencilOp::eKeep,
				.stencilCompareMask = 0uL,
				.stencilWriteMask = 0uL,
				.primitiveTopology = PrimitiveTopology::eTriangleList,
				.faceCulling = FaceCulling::eBack,
				.faceMode = FaceMode::eFill,
				.faceWinding = FaceWinding::eCcw,
				.pass = pass_.get()
			}
		)
	);

	/**/

	const auto compiler = makeVkAccCompiler();
	auto result = compiler->compile(
		{
			.effect = std::move(effect_),
			.profile = std::move(profile),
			.spec = std::move(spec)
		}
	);

	return result;
}

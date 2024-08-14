#include "SkyBoxPass.hpp"

#include <ranges>
#include <Engine.Core/Engine.hpp>
#include <Engine.Driver.Vulkan/VkRCmdTranslator.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Accel.Compile/Profile/EffectProfile.hpp>
#include <Engine.Accel.Compile/Spec/SimpleEffectSpecification.hpp>
#include <Engine.Accel.Pass/VkAccelerationPassFactory.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneDepth.hpp>
#include <Engine.GFX.RenderGraph/Symbol/ScopedSymbolContext.hpp>
#include <Engine.GFX/Scene/SkyboxModel.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBuffer.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Accel.Pipeline/GraphicsPipeline.hpp>
#include <Engine.Accel.Compile/VkEffectCompiler.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneView.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>
#include <Engine.Reflect/ExactType.hpp>
#include <Engine.GFX/Texture/TextureView.hpp>
#include <Engine.GFX/Texture/SparseTextureView.hpp>
#include <Engine.GFX/Texture/TextureFactory.hpp>
#include <Engine.Render.Scene/RenderSceneSystem.hpp>

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

void SkyBoxPass::setup(ref<graph::ScopedSymbolContext> symCtx_) noexcept {
	SubPass::setup(symCtx_);
}

void SkyBoxPass::destroy() noexcept {
	SubPass::destroy();

	auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();

	device->vkDevice().destroySemaphore(std::exchange(_tmpSignal, nullptr));

	if (_cameraBuffer.memory) {
		_cameraBuffer.unmap();
	}
	_cameraBuffer.destroy();

	_sampler->destroy();
	_sampler.reset();

	_framebuffer->destroy();
	_framebuffer.reset();

	_compiled.pipeline.reset();
	_pass.reset();
	_materialEffect.effect.reset();
}

void SkyBoxPass::declareTransforms(ref<graph::ScopedSymbolContext> symCtx_) noexcept {
	symCtx_.registerImportSymbol(makeSceneColorSymbol(), &_resources.inOutSceneColor);
	symCtx_.registerExportSymbol(makeSceneColorSymbol(), &_resources.inOutSceneColor);
}

void SkyBoxPass::declareInputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept {
	symCtx_.registerImportSymbol(makeSceneDepthSymbol(), &_resources.inSceneDepth);
}

void SkyBoxPass::iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

	if (_pass.empty()) {
		_pass = build_test_pass();
	}

	/**/

	ptr<const SkyboxModel> model = nullptr;
	auto sceneViewRes = symCtx_.getImportSymbol(makeSceneViewSymbol());
	auto sceneView = sceneViewRes->load<smr<const gfx::scene::SceneView>>();

	const auto sys = sceneView->getRenderSceneSystem();
	sys->getRegistry().forEach<SkyboxModel>(
		[&model](const auto& model_) {
			model = std::addressof(model_);
		}
	);

	/**/

	if (model == nullptr) {
		return;
	}

	/**/

	auto material = model->material(0uL);
	auto guard = material->acquire(resource::ResourceUsageFlag::eRead);

	auto proto = guard->getPrototype();
	assert(proto->getMaterialEffects().size() == 1uLL);

	auto recompile = _materialEffect.effect != proto->getMaterialEffects().front().effect;
	if (recompile) {
		_material = clone(material);

		_compiled.pipeline.reset();
		_compiled.flag = EffectCompileResultFlag::eUnknown;

		_materialEffect = clone(proto->getMaterialEffects().front());
	}
}

void SkyBoxPass::resolve() noexcept {
	if (_materialEffect.effect && _compiled.flag == EffectCompileResultFlag::eUnknown) {
		_compiled = build_test_pipeline(clone(_materialEffect.effect), clone(_pass));
	}
}

void SkyBoxPass::execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

	if (!_materialEffect.effect || !_compiled.pipeline) {
		return;
	}

	/**/

	auto sceneColorRes = symCtx_.getExportSymbol(makeSceneColorSymbol());
	auto sceneDepthRes = symCtx_.getImportSymbol(makeSceneDepthSymbol());

	const auto& sceneColorTex = sceneColorRes->load<smr<const TextureLikeObject>>();
	const auto& sceneDepthTex = sceneDepthRes->load<smr<const TextureLikeObject>>();

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

		const Vector<smr<const graph::Symbol>> outputSymbols { makeSceneColorSymbol() };
		_framebuffer = make_smr<Framebuffer>(Engine::getEngine()->getGraphics()->getCurrentDevice());

		for (const auto& output : outputSymbols) {

			auto data = symCtx_.getExportSymbol(clone(output));

			const auto& texture = data->load<smr<TextureLikeObject>>();
			_framebuffer->addAttachment(clone(texture));
		} {
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
			*device->allocator(),
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
		_cameraBuffer.map();
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

	//tmp.proj = math::perspective(glm::radians(45.F), 16.F / 9.F, 0.1F, 1000.F);

	_cameraBuffer.write<TmpStruct>(&tmp, 1uL);

	if (_cameraBufferView == nullptr) {
		_cameraBufferView = make_uptr<UniformBufferView>();
		_cameraBufferView->storeBuffer(&_cameraBuffer);
	}

	/**/

	cmd::RenderCommandBuffer cmd {};
	cmd.begin();
	cmd.beginAccelPass({ _pass.get(), _framebuffer.get() });
	cmd.beginSubPass({});

	cmd.bindGraphicsPipeline(clone(_compiled.pipeline).into<GraphicsPipeline>());

	const auto* const gpipe = static_cast<const ptr<const GraphicsPipeline>>(_compiled.pipeline.get());
	const auto& alias = _compiled.alias;

	for (const auto& group : gpipe->getBindingLayout().groups) {
		for (const auto& element : group.elements) {

			const auto symbolId = element.symbol->symbolId;
			const auto aliasId = alias.aliasOrValue(symbolId);

			static auto viewSym = lang::SymbolId::from("view"sv);
			if (aliasId == viewSym) {
				cmd.bindUniform(clone(symbolId), _cameraBufferView.get());
				continue;
			}

			static auto matSym = lang::SymbolId::from(/*"skybox"*/"mat-static-0"sv);
			if (aliasId == matSym) {

				const auto iter = std::ranges::find(
					_materialEffect.pattern->st.pairs,
					aliasId,
					[](const auto& pair_) {
						return pair_.second;
					}
				);

				if (iter == _materialEffect.pattern->st.pairs.end()) {
					assert(false);
				}

				const auto materialGuard = _material->acquire(resource::ResourceUsageFlag::eRead);
				const auto texture = materialGuard->getParam<smr<TextureResource>>(iter->first);
				const auto textureGuard = texture->acquire(resource::ResourceUsageFlag::eRead);

				/**/

				if (auto textureView = Cast<TextureView>(textureGuard.imm()->get())) {
					cmd.bindTexture(clone(symbolId), textureView);

				} else if (auto sparseTextureView = Cast<SparseTextureView>(textureGuard.imm()->get())) {

					if (sparseTextureView->vkImageView() == nullptr) {
						TextureFactory::get()->buildView(*sparseTextureView, TextureViewBuildOptions {});
					}

					cmd.bindTexture(clone(symbolId), sparseTextureView);

				} else {
					assert(false);
				}

				continue;
			}

			static auto mat0SamplerSym = lang::SymbolId::from(/*"skybox"*/"mat-static-0-sampler"sv);
			if (aliasId == mat0SamplerSym) {
				cmd.bindTextureSampler(clone(symbolId), _sampler.get());
				continue;
			}

			::hg::breakpoint();
		}

	}

	cmd.drawDispatch(1uL, 0uL, 36uL, 0uL);

	cmd.endSubPass();
	cmd.endAccelPass();
	cmd.end();

	/**/

	auto translator = make_uptr<driver::vk::VkRCmdTranslator>();
	auto nativeBatch = (*translator)(&cmd);
	const auto batch = static_cast<ptr<driver::vk::VkNativeBatch>>(nativeBatch.get());

	/**/

	{
		batch->_tmpWaits.append_range(
			reinterpret_cast<Vector<VkSemaphore>&>(sceneColorRes->barriers)
		);
		for (auto i = batch->_tmpWaitFlags.size(); i < batch->_tmpWaits.size(); ++i) {
			batch->_tmpWaitFlags.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		}
		batch->_tmpSignals.push_back(_tmpSignal);

		sceneColorRes->barriers.clear();
		sceneColorRes->barriers.push_back(_tmpSignal.operator VkSemaphore());

		/*
		batch->_tmpWaits.insert_range(
		    batch->_tmpWaits.end(),
		    reinterpret_cast<Vector<VkSemaphore>&>(sceneDepthRes->barriers)
		);
		for (auto i = batch->_tmpWaitFlags.size(); i < batch->_tmpWaits.size(); ++i) {
		    batch->_tmpWaitFlags.push_back(
		        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT
		    );
		}
		 */
	}

	batch->commitAndDispose();
	nativeBatch.reset();
}

/**/

smr<const GraphicsPass> build_test_pass() {

	constexpr auto passFactory = VkAccelerationPassFactory {};
	auto graphicsPass = passFactory.buildGraphicsPass(
		{ makeSceneColorSymbol() },
		{ makeSceneColorSymbol() }
	).value();

	return graphicsPass;
}

EffectCompileResult build_test_pipeline(
	mref<smr<const AccelerationEffect>> effect_,
	mref<smr<const GraphicsPass>> pass_
) {

	auto profile = make_smr<EffectProfile>(
		EffectProfile {
			._name = "Test-Sky-Profile",
			._definitions = {}
		}
	);

	/**/

	auto spec = make_smr<SimpleEffectSpecification>(
		Vector<smr<const lang::Symbol>> {}
	);
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

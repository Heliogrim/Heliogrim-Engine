#include "PostProcessPass.hpp"

#include <utility>
#include <Engine.Accel.Command/CommandBuffer.hpp>
#include <Engine.Accel.Compile/EffectCompileResult.hpp>
#include <Engine.Accel.Compile/VkEffectCompiler.hpp>
#include <Engine.Accel.Compile/Profile/EffectProfile.hpp>
#include <Engine.Accel.Compile/Spec/SimpleEffectSpecification.hpp>
#include <Engine.Accel.Pass/VkAccelerationPassFactory.hpp>
#include <Engine.Accel.Pipeline/GraphicsPipeline.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Driver.Vulkan/VkRCmdTranslator.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.RenderGraph/Symbol/ScopedSymbolContext.hpp>
#include <Engine.GFX/Texture/Texture.hpp>
#include <Engine.GFX/Texture/TextureFactory.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBuffer.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/Cast.hpp>

#include "__tmp_helper.hpp"

using namespace hg::engine::render;
using namespace hg::engine::accel;
using namespace hg::engine::gfx;
using namespace hg;

[[nodiscard]] static smr<AccelerationEffect> build_test_effect();

[[nodiscard]] static smr<const GraphicsPass> build_test_pass();

[[nodiscard]] static EffectCompileResult build_test_pipeline(
	mref<smr<const AccelerationEffect>> effect_,
	mref<smr<const GraphicsPass>> pass_
);

/**/

void PostProcessPass::destroy() noexcept {
	SubPass::destroy();

	auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();
	device->vkDevice().destroySemaphore(std::exchange(_tmpSignal, nullptr));

	_colorView.reset();
	_colorTexture->destroy();
	_colorTexture.reset();

	_sampler->destroy();
	_sampler.reset();

	_framebuffer->destroy();
	_framebuffer.reset();

	_compiled.pipeline.reset();
	_pass.reset();
	_effect.reset();
}

void PostProcessPass::declareTransforms(ref<graph::ScopedSymbolContext> symCtx_) noexcept {
	SubPass::declareTransforms(symCtx_);

	symCtx_.registerImportSymbol(makeSceneColorSymbol(), &_resources.inOutSceneColor);
	symCtx_.registerExportSymbol(makeSceneColorSymbol(), &_resources.inOutSceneColor);
}

void PostProcessPass::iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

	if (_effect.empty()) {
		_effect = build_test_effect();
	}

	if (_pass.empty()) {
		_pass = build_test_pass();
	}
}

void PostProcessPass::resolve() noexcept {
	if (_effect && _compiled.flag == accel::EffectCompileResultFlag::eUnknown) {
		_compiled = build_test_pipeline(clone(_effect), clone(_pass));
	}
}

void PostProcessPass::execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

	assert(_effect);
	assert(_compiled.pipeline);

	/**/

	auto sceneColorRes = symCtx_.getExportSymbol(makeSceneColorSymbol());
	const auto& sceneColorTex = sceneColorRes->load<smr<const TextureLikeObject>>();

	if (not _framebuffer.empty() && (_framebuffer->attachments().front() != sceneColorTex)) {

		_colorView.reset();
		_colorTexture->destroy();
		_colorTexture.reset();

		_framebuffer->device()->vkDevice().destroySemaphore(_tmpSignal);
		_framebuffer->destroy();
		_framebuffer.reset();
	}

	if (_framebuffer.empty()) {

		const auto* const origin = Cast<Texture>(sceneColorTex.get());

		auto* tf = TextureFactory::get();
		const auto payload = TextureBuildPayload {
			.extent = origin->extent(),
			.format = origin->format(),
			.mipLevels = origin->mipLevels(),
			.type = origin->type(),
			.vkAspect = origin->buffer()._vkAspect,
			.vkUsage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			.vkMemoryFlags = vk::MemoryPropertyFlagBits::eDeviceLocal,
			.vkSharing = vk::SharingMode::eExclusive
		};
		_colorTexture = make_uptr<gfx::Texture>(tf->build(payload));
		tf->buildView(*_colorTexture);

		_colorView = _colorTexture->makeView(
			math::uivec2 { _colorTexture->layer() },
			math::uExtent3D { _colorTexture->width(), _colorTexture->height(), _colorTexture->depth() },
			{ 0uL, _colorTexture->mipLevels() }
		);
		_colorView->owner()->buffer()._vkLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

		/**/

		_framebuffer = make_smr<Framebuffer>(Engine::getEngine()->getGraphics()->getCurrentDevice());

		_framebuffer->addAttachment(clone(sceneColorRes->load<smr<TextureLikeObject>>()));
		_framebuffer->setExtent(Cast<Texture>(sceneColorTex.get())->extent());

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

	cmd::RenderCommandBuffer cmd {};
	cmd.begin();

	cmd.lambda(
		[sceneColorTex, colorTexture = _colorTexture.get()](ref<AccelCommandBuffer> cmd_) {
			const auto* const texture = Cast<gfx::Texture>(sceneColorTex.get());

			std::array<vk::ImageMemoryBarrier, 2> barriers = {
				vk::ImageMemoryBarrier {
					vk::AccessFlags(),
					vk::AccessFlags(),
					vk::ImageLayout::eColorAttachmentOptimal,
					vk::ImageLayout::eTransferSrcOptimal,
					VK_QUEUE_FAMILY_IGNORED,
					VK_QUEUE_FAMILY_IGNORED,
					texture->buffer().image(),
					vk::ImageSubresourceRange {
						vk::ImageAspectFlagBits::eColor,
						0uL,
						texture->mipLevels(),
						0uL,
						1uL
					}
				},
				vk::ImageMemoryBarrier {
					vk::AccessFlags(),
					vk::AccessFlags(),
					vk::ImageLayout::eUndefined,
					vk::ImageLayout::eTransferDstOptimal,
					VK_QUEUE_FAMILY_IGNORED,
					VK_QUEUE_FAMILY_IGNORED,
					colorTexture->buffer().image(),
					vk::ImageSubresourceRange {
						vk::ImageAspectFlagBits::eColor,
						0uL,
						colorTexture->mipLevels(),
						0uL,
						1uL
					}
				}
			};

			cmd_.vkCommandBuffer().pipelineBarrier(
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::PipelineStageFlagBits::eTransfer,
				vk::DependencyFlags(),
				0uL,
				nullptr,
				0uL,
				nullptr,
				static_cast<u32>(barriers.size()),
				barriers.data()
			);

			const vk::ImageCopy copyRegion {
				vk::ImageSubresourceLayers {
					vk::ImageAspectFlagBits::eColor, 0uL, 0uL, 1uL
				},
				vk::Offset3D { 0L, 0L, 0L },
				vk::ImageSubresourceLayers {
					vk::ImageAspectFlagBits::eColor, 0uL, 0uL, 1uL
				},
				vk::Offset3D { 0L, 0L, 0L },
				vk::Extent3D { texture->width(), texture->height(), texture->depth() }
			};
			cmd_.vkCommandBuffer().copyImage(
				texture->buffer().image(),
				vk::ImageLayout::eTransferSrcOptimal,
				colorTexture->buffer().image(),
				vk::ImageLayout::eTransferDstOptimal,
				1,
				&copyRegion
			);

			std::array<vk::ImageMemoryBarrier, 2> inverse = {
				vk::ImageMemoryBarrier {
					vk::AccessFlags(),
					vk::AccessFlags(),
					vk::ImageLayout::eTransferSrcOptimal,
					vk::ImageLayout::eColorAttachmentOptimal,
					VK_QUEUE_FAMILY_IGNORED,
					VK_QUEUE_FAMILY_IGNORED,
					texture->buffer().image(),
					vk::ImageSubresourceRange {
						vk::ImageAspectFlagBits::eColor,
						0uL,
						texture->mipLevels(),
						0uL,
						1uL
					}
				},
				vk::ImageMemoryBarrier {
					vk::AccessFlags(),
					vk::AccessFlags(),
					vk::ImageLayout::eTransferDstOptimal,
					vk::ImageLayout::eShaderReadOnlyOptimal,
					VK_QUEUE_FAMILY_IGNORED,
					VK_QUEUE_FAMILY_IGNORED,
					colorTexture->buffer().image(),
					vk::ImageSubresourceRange {
						vk::ImageAspectFlagBits::eColor,
						0uL,
						colorTexture->mipLevels(),
						0uL,
						1uL
					}
				}
			};

			cmd_.vkCommandBuffer().pipelineBarrier(
				vk::PipelineStageFlagBits::eTransfer,
				vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::DependencyFlags(),
				0uL,
				nullptr,
				0uL,
				nullptr,
				static_cast<u32>(inverse.size()),
				inverse.data()
			);
		}
	);

	cmd.beginAccelPass({ _pass.get(), _framebuffer.get() });
	cmd.beginSubPass();

	cmd.bindGraphicsPipeline(clone(_compiled.pipeline).into<GraphicsPipeline>());

	const auto* const gpipe = Cast<GraphicsPipeline>(_compiled.pipeline.get());
	const auto& alias = _compiled.alias;

	for (const auto& group : gpipe->getBindingLayout().groups) {
		for (const auto& element : group.elements) {

			const auto symbolId = element.symbol->symbolId;
			const auto aliasId = alias.aliasOrValue(symbolId);

			static auto matSym = lang::SymbolId::from(/*"sampled"*/"color-tex-0"sv);
			if (aliasId == matSym) {
				cmd.bindTexture(clone(symbolId), _colorView.get());
				continue;
			}

			::hg::breakpoint();
		}
	}

	cmd.drawDispatch(1uL, 0uL, 6uL, 0uL);

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
			batch->_tmpWaitFlags.emplace_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		}
		batch->_tmpSignals.push_back(_tmpSignal);

		sceneColorRes->barriers.clear();
		sceneColorRes->barriers.emplace_back(_tmpSignal.operator VkSemaphore());
	}

	nativeBatch->commitAndDispose();
	nativeBatch.reset();
}

/**/

smr<AccelerationEffect> build_test_effect() {

	auto vertexStage = make_smr<Stage>(StageFlagBits::eVertex);
	auto fragmentStage = make_smr<Stage>(StageFlagBits::eFragment);

	const auto vertexShaderCode = read_shader_file("__test__pp.vs");
	const auto fragmentShaderCode = read_shader_file("__test__pp.fs");

	vertexStage->setIntermediate(make_smr<lang::Intermediate>());
	vertexStage->getIntermediate()->lang.dialect = lang::Dialect::eVulkanGlsl460;
	vertexStage->getIntermediate()->lang.text.emplace_back(std::move(vertexShaderCode));

	fragmentStage->setIntermediate(make_smr<lang::Intermediate>());
	fragmentStage->getIntermediate()->lang.dialect = lang::Dialect::eVulkanGlsl460;
	fragmentStage->getIntermediate()->lang.text.emplace_back(std::move(fragmentShaderCode));

	/**/

	using namespace ::hg::engine::accel::lang;

	uptr<Variable> tmpVar {};
	uptr<Symbol> tmpSym {};

	tmpVar = make_uptr<Variable>();
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eUniform>>(std::move(tmpVar->annotation));
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("color-tex-0", std::move(tmpVar->annotation));
	tmpVar->type = Type {
		.category = TypeCategory::eTexture, .textureType = lang::TextureType::eTexture2d
	};

	tmpSym = make_uptr<Symbol>(
		SymbolId::from("color-tex-0"sv),
		VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
	);

	fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
	fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));

	/**/

	{
		auto tmpVar = make_uptr<Variable>();
		tmpVar->type = Type { .category = TypeCategory::eObject, .objectType = ObjectType::eSampler };
		tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eExternalLinkage>>();
		tmpVar->annotation = make_uptr<SymbolIdAnnotation>("color-tex-0-sampler", std::move(tmpVar->annotation));
		tmpSym = make_uptr<Symbol>(
			SymbolId::from("color-tex-0-sampler"sv),
			VariableSymbol { SymbolType::eVariableSymbol, tmpVar.get() }
		);
		fragmentStage->getIntermediate()->rep.globalScope.inbound.emplace_back(std::move(tmpVar));
		fragmentStage->getIntermediate()->rep.symbolTable.insert(std::move(tmpSym));
	}

	/**/

	tmpVar = make_uptr<Variable>();
	tmpVar->annotation = make_uptr<SimpleAnnotation<AnnotationType::eForwardLinkage>>();
	tmpVar->annotation = make_uptr<SymbolIdAnnotation>("color", std::move(tmpVar->annotation));
	tmpVar->type = Type {
		.category = TypeCategory::eTexture, .textureType = lang::TextureType::eTexture2d
	};

	fragmentStage->getIntermediate()->rep.globalScope.outbound.emplace_back(std::move(tmpVar));

	/**/

	Guid guid { static_cast<u32>("__Test__Proxy"_typeId.data), 0, 0, "PostProcessEffect"_typeId.data };

	return make_smr<AccelerationEffect>(
		std::move(guid),
		"test-pp-effect",
		std::vector { std::move(vertexStage), std::move(fragmentStage) }
	);
}

smr<const GraphicsPass> build_test_pass() {

	constexpr auto passFactory = VkAccelerationPassFactory();
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
		"Test-PostProcess-Profile",
		Vector<ProfileDefinition> {}
	);

	/**/

	auto spec = make_smr<SimpleEffectSpecification>();
	spec->setPassSpec(
		make_uptr<GraphicsPassSpecification>(
			DepthCompareOp::eNever,
			DepthBias {},
			StencilCompareOp::eNever,
			StencilOp::eKeep,
			StencilOp::eKeep,
			StencilOp::eKeep,
			0uL,
			0uL,
			PrimitiveTopology::eTriangleList,
			FaceCulling::eNone,
			FaceMode::eFill,
			FaceWinding::eCcw,
			pass_.get()
		)
	);

	/**/

	const auto compiler = makeVkAccCompiler();
	auto result = compiler->compile(
		{
			std::move(effect_),
			std::move(profile),
			std::move(spec)
		}
	);
	return result;
}

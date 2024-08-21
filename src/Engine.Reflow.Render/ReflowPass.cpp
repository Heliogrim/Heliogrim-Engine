#include "ReflowPass.hpp"

#include <cassert>
#include <cstring>
#include <utility>
#include <Engine.Accel.Command/CommandBuffer.hpp>
#include <Engine.Accel.Compile/VkEffectCompiler.hpp>
#include <Engine.Accel.Compile/Profile/EffectProfile.hpp>
#include <Engine.Accel.Compile/Spec/SimpleEffectSpecification.hpp>
#include <Engine.Accel.Pass/VkAccelerationPassFactory.hpp>
#include <Engine.Accel.Pipeline/GraphicsPipeline.hpp>
#include <Engine.Accel.Pipeline/VkGraphicsPipeline.hpp>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Assets/Types/Image.hpp>
#include <Engine.Assets/Types/Texture/TextureAsset.hpp>
#include <Engine.Assets/Types/Texture/TextureAsset.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Driver.Vulkan/VkRCmdTranslator.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBuffer.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX.Loader/Texture/TextureLoader.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.GFX.Render.Predefined/Effects/UIBase.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneDepth.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneView.hpp>
#include <Engine.GFX.RenderGraph/Relation/TextureDescription.hpp>
#include <Engine.GFX.RenderGraph/Symbol/ScopedSymbolContext.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>
#include <Engine.GFX/Buffer/Buffer.hpp>
#include <Engine.GFX/Texture/SparseTextureView.hpp>
#include <Engine.GFX/Texture/Texture.hpp>
#include <Engine.GFX/Texture/TextureFactory.hpp>
#include <Engine.GFX/Texture/TextureView.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Reflect/ExactType.hpp>
#include <Engine.Reflect/TypeSwitch.hpp>
#include <Engine.Reflow/FlowContext.hpp>
#include <Engine.Reflow/ReflowEngine.hpp>
#include <Engine.Reflow/Command/ReflowCommandBuffer.hpp>
#include <Engine.Reflow/Scene/UISceneModel.hpp>
#include <Engine.Reflow/Window/Window.hpp>
#include <Engine.Render.Scene/RenderSceneSystem.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Heliogrim/TextureFormat.hpp>
#include <Heliogrim/TextureType.hpp>

/**/

using namespace hg::engine::reflow;
using namespace hg::engine::render;
using namespace hg;

[[nodiscard]] static engine::accel::EffectCompileResult build_test_opaque_pipeline(
	mref<smr<const engine::accel::GraphicsPass>> pass_
);

[[nodiscard]] static engine::accel::EffectCompileResult build_test_alpha_pipeline(
	mref<smr<const engine::accel::GraphicsPass>> pass_
);

[[nodiscard]] static smr<const graph::Description> makeLocalDepthDescription();

[[nodiscard]] static smr<graph::Symbol> makeLocalDepthSymbol();

/**/

render::ReflowPass::ReflowPass() = default;

render::ReflowPass::~ReflowPass() = default;

void render::ReflowPass::destroy() noexcept {
	SubPass::destroy();

	const auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();
	device->vkDevice().destroySemaphore(std::exchange(_tmpSignal, nullptr));

	if (_framebuffer) {
		_framebuffer->destroy();
		_framebuffer.reset();
	}

	if (_depthImage) {
		_depthImage->destroy();
		_depthImage.reset();
	}

	if (_uiVertexBuffer) {
		Cast<gfx::Buffer>(_uiVertexBuffer.get())->destroy();
		_uiVertexBuffer.reset();
	}

	if (_uiIndexBuffer) {
		Cast<gfx::Buffer>(_uiIndexBuffer.get())->destroy();
		_uiIndexBuffer.reset();
	}

	_opaqueSubPass.imageSampler.reset();
	_alphaSubPass.imageSampler.reset();
	_msdfSubPass.imageSampler.reset();

	_opaqueSubPass.compiled.pipeline.reset();
	_alphaSubPass.compiled.pipeline.reset();
	_msdfSubPass.compiled.pipeline.reset();

	_graphicsPass.reset();
}

void render::ReflowPass::iterate(cref<engine::render::graph::ScopedSymbolContext> symCtx_) noexcept {

	ensureGraphicsPass();

}

void render::ReflowPass::resolve() noexcept {

	if (_opaqueSubPass.compiled.flag == accel::EffectCompileResultFlag::eUnknown) {
		_opaqueSubPass.compiled = build_test_opaque_pipeline(clone(_graphicsPass));

		if (_opaqueSubPass.imageSampler == nullptr) {
			_opaqueSubPass.imageSampler = make_uptr<gfx::TextureSampler>();
			_opaqueSubPass.imageSampler->setup(Engine::getEngine()->getGraphics()->getCurrentDevice());
		}
	}

	if (_alphaSubPass.compiled.flag == accel::EffectCompileResultFlag::eUnknown) {
		_alphaSubPass.compiled = build_test_alpha_pipeline(clone(_graphicsPass));

		if (_alphaSubPass.imageSampler == nullptr) {
			_alphaSubPass.imageSampler = make_uptr<gfx::TextureSampler>();
			_alphaSubPass.imageSampler->setup(Engine::getEngine()->getGraphics()->getCurrentDevice());
		}
	}

	if (_msdfSubPass.compiled.flag == accel::EffectCompileResultFlag::eUnknown) {
		//_msdfSubPass.compiled = build_test_msdf_pipeline(clone(_graphicsPass));

		if (_msdfSubPass.imageSampler == nullptr) {
			_msdfSubPass.imageSampler = make_uptr<gfx::TextureSampler>();
			_msdfSubPass.imageSampler->setup(Engine::getEngine()->getGraphics()->getCurrentDevice());
		}
	}
}

void render::ReflowPass::execute(cref<engine::render::graph::ScopedSymbolContext> symCtx_) noexcept {

	const auto sceneColor = symCtx_.getExportSymbol(makeSceneColorSymbol());
	const auto sceneColorRes = sceneColor->load<smr<gfx::TextureLikeObject>>();
	const auto sceneColorTex = Cast<gfx::Texture>(sceneColorRes.get());

	/**/

	assert(not sceneColor->empty());

	if (shouldChangeFrame(sceneColorRes)) {
		ensureDepthBuffer(*sceneColorRes);
		ensureFramebuffer(clone(sceneColorRes));
	}

	/**/

	ptr<const UISceneModel> uiModel = nullptr; {
		auto sceneViewRes = symCtx_.getImportSymbol(makeSceneViewSymbol());
		auto sceneView = sceneViewRes->load<smr<const gfx::scene::SceneView>>();

		const auto sys = sceneView->getRenderSceneSystem();
		sys->getRegistry().forEach<UISceneModel>(
			[&uiModel](const auto& model_) {
				uiModel = std::addressof(model_);
			}
		);
	}

	if (uiModel == nullptr) {
		return;
	}

	/**/

	ReflowCommandBuffer uiCmd {};
	uiCmd._opaque.indices.reserve(16uLL * 1024uLL);
	uiCmd._opaque.vertices.reserve(16uLL * 1024uLL);
	uiCmd._alpha.indices.reserve(16uLL * 1024uLL);
	uiCmd._alpha.vertices.reserve(16uLL * 1024uLL);

	math::vec2 available {
		static_cast<float>(sceneColorTex->width()),
		static_cast<float>(sceneColorTex->height())
	};
	const FlowContext fctx {
		math::fExtent2D { available.x, available.y, 0.F, 0.F },
		math::fExtent2D { available.x, available.y, 0.F, 0.F }
	};

	const math::fExtent2D rootScissor { fctx.scissor };
	uiCmd.pushRootScissor(rootScissor);

	/**/

	auto& wnd = uiModel->getWindow();
	//wnd.render(&uiCmd);

	{
		const auto poppedRoot = uiCmd.popScissor();
		assert(rootScissor == poppedRoot);
	}

	/**/

	const auto globalRenderTick = ReflowEngine::getGlobalRenderTick();
	if (_lastRenderTick > globalRenderTick || (globalRenderTick - _lastRenderTick) > 56u) {
		/* If render tick rolled or diff is too large, force a full redraw */
		_lastRenderTick = 0u;
	}

	auto markForCapture = Vector<nmpt<const Widget>> {};
	wnd.enumerateDistinctCapture(_lastRenderTick, markForCapture);

	_lastRenderTick = globalRenderTick;

	/**/

	for (auto marked : markForCapture) {

		if (marked->layoutState().layoutSize.anyZero()) {
			continue;
		}

		/**/

		const math::fExtent2D markScissor {
			marked->layoutState().layoutSize.x,
			marked->layoutState().layoutSize.y,
			marked->layoutState().layoutOffset.x,
			marked->layoutState().layoutOffset.y
		};

		uiCmd.pushScissor(markScissor);
		wnd.render(&uiCmd);
		[[maybe_unused]] auto poppedRoot = uiCmd.popScissor();

		assert(poppedRoot == markScissor);
	}

	/**/

	if (uiCmd._alpha.vertices.empty() && uiCmd._opaque.vertices.empty()) {
		return;
	}

	updateVertices(uiCmd._opaque.vertices, uiCmd._alpha.vertices, {});
	updateIndices(uiCmd._opaque.indices, uiCmd._alpha.indices, {});

	/**/

	cmd::RenderCommandBuffer cmd {};

	cmd.begin();
	cmd.beginAccelPass(
		{
			.pass = _graphicsPass.get(), .framebuffer = _framebuffer.get(),
			.clearValues = { vk::ClearColorValue { 0.F, 0.F, 0.F, 1.F }, vk::ClearDepthStencilValue { 0.F, 0uL } }
		}
	);
	cmd.beginSubPass();

	/**/

	cmd.bindGraphicsPipeline(clone(_opaqueSubPass.compiled.pipeline).into<accel::GraphicsPipeline>());
	cmd.bindTexture(accel::lang::SymbolId::from("depth"sv), _depthImage.get());

	captureOpaque(uiCmd, rootScissor, cmd);

	/**/

	//cmd.nextSubPass();

	/**/

	cmd.bindGraphicsPipeline(clone(_alphaSubPass.compiled.pipeline).into<accel::GraphicsPipeline>());
	cmd.bindTexture(accel::lang::SymbolId::from("depth"sv), _depthImage.get());

	captureAlpha(uiCmd, rootScissor, cmd);

	/**/

	cmd.endSubPass();
	cmd.endAccelPass();
	cmd.end();

	/**/

	const auto translator = make_uptr<driver::vk::VkRCmdTranslator>();
	auto prevNativeBatch = std::move(_nativeBatch);

	auto nextNativeBatch = (*translator)(&cmd, std::move(prevNativeBatch));

	/**/

	{
		const auto batch = static_cast<ptr<driver::vk::VkNativeBatch>>(nextNativeBatch.get());

		batch->_tmpWaits.append_range(
			reinterpret_cast<Vector<VkSemaphore>&>(sceneColor->barriers)
		);
		batch->_tmpWaits.append_range(
			reinterpret_cast<Vector<VkSemaphore>&>(uiCmd._imageWait)
		);

		for (auto i = batch->_tmpWaitFlags.size(); i < batch->_tmpWaits.size(); ++i) {
			batch->_tmpWaitFlags.emplace_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		}

		batch->_tmpSignals.emplace_back(_tmpSignal);
		batch->_tmpSignals.append_range(
			reinterpret_cast<Vector<VkSemaphore>&>(uiCmd._imageSignal)
		);

		/**/

		sceneColor->barriers.clear();
		sceneColor->barriers.emplace_back(_tmpSignal.operator VkSemaphore());
	}

	nextNativeBatch->commitAndDispose();

	/**/

	_nativeBatch = std::move(nextNativeBatch);
}

void render::ReflowPass::ensureDefaultImage() {

	const auto factory = Engine::getEngine()->getAssets()->getFactory();
	const auto registry = Engine::getEngine()->getAssets()->getRegistry();

	// TODO: Rework temporary solution
	constexpr auto default_ui_asset = asset_guid { 2893474171, 5859, 16540, 8673728298833701775uLL };
	const auto asset = registry->findAssetByGuid(default_ui_asset);
	::hg::assertrt(asset != nullptr);

	/**/

	auto request = static_cast<ptr<assets::TextureAsset>>(asset.get());
	auto resource = Engine::getEngine()->getResources()->loader().load<assets::TextureAsset, gfx::TextureResource>(
		std::move(request)
	);

	auto guard = resource->acquire(resource::ResourceUsageFlag::eAll);
	auto* const image = Cast<gfx::SparseTextureView>(guard->get());
	assert(image);

	if (not image->vkImageView()) {
		engine::gfx::TextureFactory::get()->buildView(*image, { .type = TextureType::e2d });
	}

	_defaultImage = std::move(resource);
}

void render::ReflowPass::ensureGraphicsPass() {

	if (not _graphicsPass.empty()) {
		return;
	}

	constexpr auto factory = accel::VkAccelerationPassFactory();
	_graphicsPass = factory.buildGraphicsPass(
		{
			makeSceneColorSymbol(),
			makeLocalDepthSymbol()
		},
		{
			makeSceneColorSymbol()
		}
	).value();
}

bool render::ReflowPass::shouldChangeFrame(cref<smr<gfx::TextureLikeObject>> colorTarget_) const noexcept {

	if (_framebuffer == nullptr) {
		return true;
	}

	if (_depthImage == nullptr) {
		return true;
	}

	if (_framebuffer->attachments().front() != colorTarget_) {
		return true;
	}

	return false;
}

void render::ReflowPass::ensureDepthBuffer(cref<gfx::TextureLikeObject> colorTarget_) {

	if (_depthImage != nullptr) {
		_depthImage.reset();
	}

	/**/

	const auto* const factory = gfx::TextureFactory::get();
	auto texture = switchType(
		std::addressof(colorTarget_),
		[factory](const ptr<const gfx::Texture> texture_) {
			return factory->build(
				{
					math::uivec3 { texture_->width(), texture_->height(), 1uL },
					TextureFormat::eD16Unorm,
					1uL,
					TextureType::e2d,
					vk::ImageAspectFlagBits::eDepth,
					vk::ImageUsageFlagBits::eDepthStencilAttachment,
					vk::MemoryPropertyFlagBits::eDeviceLocal,
					vk::SharingMode::eExclusive
				}
			);
		}
	);

	::hg::assertrt(
		[&texture]() {
			return std::addressof(texture) != nullptr;
		}
	);

	factory->buildView(texture, { .layers = { 0L, 1L }, .type = TextureType::e2d, .mipLevels = { 0L, 1L } });
	_depthImage = make_smr<gfx::Texture>(std::move(texture));
}

void render::ReflowPass::ensureFramebuffer(mref<smr<gfx::TextureLikeObject>> colorTarget_) {

	if (_framebuffer != nullptr) {
		_framebuffer->device()->vkDevice().destroySemaphore(_tmpSignal);
		_framebuffer->destroy();
		_framebuffer.reset();
	}

	_framebuffer = make_uptr<gfx::Framebuffer>(Engine::getEngine()->getGraphics()->getCurrentDevice());

	const auto* const texture = Cast<gfx::Texture>(colorTarget_.get());
	_framebuffer->addAttachment(clone(colorTarget_));
	_framebuffer->addAttachment(clone(_depthImage));

	_framebuffer->setExtent(texture->extent());
	_framebuffer->setRenderPass(clone(_graphicsPass));

	_framebuffer->setup();
	_tmpSignal = _framebuffer->device()->vkDevice().createSemaphore({});
}

void render::ReflowPass::updateVertices(
	cref<std::span<gfx::uivertex>> opaque_,
	cref<std::span<gfx::uivertex>> alpha_,
	cref<std::span<gfx::uivertex>> msdf_
) {

	const auto alphaVertexSize = alpha_.size() * sizeof(gfx::uivertex);
	const auto opaqueVertexSize = opaque_.size() * sizeof(gfx::uivertex);

	const auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();
	const auto vertexSize = alphaVertexSize + opaqueVertexSize;

	if (_uiVertexBuffer == nullptr || Cast<gfx::Buffer>(_uiVertexBuffer.get())->size < vertexSize) {

		if (_uiVertexBuffer) {
			Cast<gfx::Buffer>(_uiVertexBuffer.get())->destroy();
			_uiVertexBuffer.reset();
		}

		_uiVertexBuffer = make_uptr<gfx::Buffer>();
		auto& next = *Cast<gfx::Buffer>(_uiVertexBuffer.get());

		next.size = vertexSize;
		next.usageFlags = vk::BufferUsageFlagBits::eVertexBuffer;
		next.device = device->vkDevice();

		const vk::BufferCreateInfo bci {
			vk::BufferCreateFlags {},
			next.size, next.usageFlags, vk::SharingMode::eExclusive, 0, nullptr
		};

		next.buffer = device->vkDevice().createBuffer(bci);
		assert(next.buffer);

		[[maybe_unused]] auto allocResult = gfx::memory::allocate(
			*device->allocator(),
			device,
			next.buffer,
			gfx::MemoryProperty::eHostVisible,
			next.memory
		);
		next.bind();
	}

	/**/

	ref<gfx::Buffer> target = *Cast<gfx::Buffer>(_uiVertexBuffer.get());
	target.map(vertexSize, 0uLL);

	/**/

	_opaqueSubPass.vertexOffset = 0LL;
	std::memcpy(target.memory->mapping, opaque_.data(), opaque_.size() * sizeof(gfx::uivertex));

	/**/

	_alphaSubPass.vertexOffset = _opaqueSubPass.vertexOffset + opaque_.size() * sizeof(gfx::uivertex);
	std::memcpy(
		static_cast<const ptr<char>>(target.memory->mapping) + _alphaSubPass.vertexOffset,
		alpha_.data(),
		alpha_.size() * sizeof(gfx::uivertex)
	);

	/**/

	_msdfSubPass.vertexOffset = _alphaSubPass.vertexOffset + alpha_.size() * sizeof(gfx::uivertex);
	std::memcpy(
		static_cast<const ptr<char>>(target.memory->mapping) + _msdfSubPass.vertexOffset,
		msdf_.data(),
		msdf_.size() * sizeof(gfx::uivertex)
	);

	/**/

	target.unmap();
}

void render::ReflowPass::updateIndices(
	cref<std::span<u32>> opaque_,
	cref<std::span<u32>> alpha_,
	cref<std::span<u32>> msdf_
) {

	const auto opaqueIndexSize = opaque_.size_bytes();
	const auto alphaIndexSize = alpha_.size_bytes();
	const auto msdfIndexSize = msdf_.size_bytes();

	const auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();
	const auto indexSize = opaqueIndexSize + alphaIndexSize + msdfIndexSize;

	if (_uiIndexBuffer == nullptr || Cast<gfx::Buffer>(_uiIndexBuffer.get())->size < indexSize) {

		if (_uiIndexBuffer) {
			Cast<gfx::Buffer>(_uiIndexBuffer.get())->destroy();
			_uiIndexBuffer.reset();
		}

		_uiIndexBuffer = make_uptr<gfx::Buffer>();
		auto& next = *Cast<gfx::Buffer>(_uiIndexBuffer.get());

		next.size = indexSize;
		next.usageFlags = vk::BufferUsageFlagBits::eIndexBuffer;
		next.device = device->vkDevice();

		const vk::BufferCreateInfo bci {
			vk::BufferCreateFlags {},
			next.size, next.usageFlags, vk::SharingMode::eExclusive, 0, nullptr
		};

		next.buffer = device->vkDevice().createBuffer(bci);
		assert(next.buffer);

		[[maybe_unused]] auto allocResult = gfx::memory::allocate(
			*device->allocator(),
			device,
			next.buffer,
			gfx::MemoryProperty::eHostVisible,
			next.memory
		);
		next.bind();
	}

	/**/

	auto& target = *Cast<gfx::Buffer>(_uiIndexBuffer.get());
	target.map(indexSize, 0uLL);

	/**/

	_opaqueSubPass.indexOffset = 0LL;
	_opaqueSubPass.indexSize = opaque_.size_bytes();

	std::memcpy(
		target.memory->mapping,
		opaque_.data(),
		opaque_.size_bytes()
	);

	/**/

	_alphaSubPass.indexOffset = _opaqueSubPass.indexOffset + _opaqueSubPass.indexSize;
	_alphaSubPass.indexSize = alpha_.size_bytes();

	std::memcpy(
		static_cast<const ptr<char>>(target.memory->mapping) + _alphaSubPass.indexOffset,
		alpha_.data(),
		alpha_.size_bytes()
	);

	/**/

	_msdfSubPass.indexOffset = _alphaSubPass.indexOffset + _alphaSubPass.indexSize;
	_msdfSubPass.indexSize = msdf_.size_bytes();

	std::memcpy(
		static_cast<const ptr<char>>(target.memory->mapping) + _msdfSubPass.indexOffset,
		msdf_.data(),
		msdf_.size_bytes()
	);

	/**/

	target.unmap();
}

void render::ReflowPass::captureOpaque(
	cref<ReflowCommandBuffer> records_,
	cref<math::fExtent2D> scope_,
	ref<engine::render::cmd::RenderCommandBuffer> cmd_
) {

	cmd_.lambda(
		[this, scope_](ref<accel::AccelCommandBuffer> cmd_) {

			struct UiTransformBlock {
				math::vec2 extent;
				math::vec2 center;
			};

			const auto block = UiTransformBlock {
				math::vec2 { -1.F },
				math::vec2 { 2.F / scope_.width, 2.F / scope_.height }
			};

			cmd_.vkCommandBuffer().pushConstants(
				reinterpret_cast<VkPipelineLayout>(
					Cast<accel::VkGraphicsPipeline>(_opaqueSubPass.compiled.pipeline.get())->_vkPipeLayout
				),
				vk::ShaderStageFlagBits::eVertex,
				0uL,
				sizeof(UiTransformBlock),
				&block
			);

			/**/

			auto* vb = Cast<gfx::Buffer>(_uiVertexBuffer.get());
			auto* ib = Cast<gfx::Buffer>(_uiIndexBuffer.get());

			cmd_.bindVertexBuffer(0uL, *vb, _opaqueSubPass.vertexOffset);
			cmd_.bindIndexBuffer(static_cast<ref<gfx::IndexBuffer>>(*ib), _opaqueSubPass.indexOffset);
		}
	);

	/* Bind default image */

	if (_defaultImage.empty()) {
		ensureDefaultImage();
	}

	const auto defaultImageGuard = _defaultImage->acquire(resource::ResourceUsageFlag::eRead);
	auto* const defaultImage = Cast<gfx::SparseTextureView>(defaultImageGuard->get());

	// UIDummy

	cmd_.bindTexture(accel::lang::SymbolId::from("ui-image"sv), defaultImage);
	cmd_.bindTextureSampler(accel::lang::SymbolId::from("ui-image-sampler"sv), _opaqueSubPass.imageSampler.get());

	/**/

	if (records_._scissors.empty()) {
		return;
	}

	/* Warning: Dirty Hack for Sanitizing */

	if (records_._opaque.imageSpan.empty()) {
		const_cast<Vector<ImageSpan>&>(records_._opaque.imageSpan).emplace_back(
			records_._opaque.scissorSpan.back().last,
			records_._opaque.scissorSpan.back().last,
			~0uL
		);
	}

	/**/

	const auto& firstScissorSpan = records_._opaque.scissorSpan.front();
	const auto& lastScissorSpan = records_._opaque.scissorSpan.back();

	auto scissorIt = records_._opaque.scissorSpan.begin();
	auto imageIt = records_._opaque.imageSpan.begin();

	u32 idx = firstScissorSpan.first;
	while (idx < lastScissorSpan.last) {

		const auto startIdx = idx;
		const auto endIdx = (std::min)(
			idx < scissorIt->first ? scissorIt->first : scissorIt->last,
			idx < imageIt->first ? imageIt->first : imageIt->last
		);

		const auto& scissor = records_._scissors[scissorIt->idx];

		/**/

		const auto image = [&]() -> nmpt<gfx::TextureLikeObject> {

			if (idx < imageIt->first || idx >= imageIt->last) {
				return defaultImage;
			}

			return records_._images[imageIt->idx];
		}();

		/**/

		const auto vkScissor = vk::Rect2D {
			vk::Offset2D { static_cast<s32>(scissor.offsetX), static_cast<s32>(scissor.offsetY) },
			vk::Extent2D { static_cast<u32>(scissor.width), static_cast<u32>(scissor.height) }
		};

		cmd_.lambda(
			[vkScissor](ref<accel::AccelCommandBuffer> accel_) {
				accel_.vkCommandBuffer().setScissor(0, 1, &vkScissor);
			}
		);

		/**/

		switchType(
			image.get(),
			[](const ptr<gfx::Texture> texture_) {
				if (not texture_->vkView()) {
					gfx::TextureFactory::get()->buildView(*texture_);
				}
			},
			[](const ptr<gfx::TextureView> view_) {
				if (not view_->owner()->vkView()) {
					gfx::TextureFactory::get()->buildView(*view_->owner());
				}
			},
			[](const ptr<gfx::SparseTexture> texture_) {
				if (not texture_->_vkImageView) {
					gfx::TextureFactory::get()->buildView(*texture_);
				}
			},
			[](const ptr<gfx::SparseTextureView> view_) {
				if (not view_->vkImageView()) {
					gfx::TextureFactory::get()->buildView(*view_);
				}
			}
		);

		cmd_.bindTexture(
			accel::lang::SymbolId::from("ui-image"sv),
			image.get()
		);

		/**/

		cmd_.drawStaticMeshIdx(1uL, 0uL, (endIdx - startIdx) / 3uL, startIdx / 3uL);

		/**/

		if (scissorIt->last == endIdx) {
			++scissorIt;
		}

		if (imageIt->last == endIdx) {
			++imageIt;
		}

		idx = endIdx;
	}
}

void render::ReflowPass::captureAlpha(
	cref<ReflowCommandBuffer> records_,
	cref<math::fExtent2D> scope_,
	ref<engine::render::cmd::RenderCommandBuffer> cmd_
) {

	cmd_.lambda(
		[this, scope_](ref<accel::AccelCommandBuffer> cmd_) {

			struct UiTransformBlock {
				math::vec2 extent;
				math::vec2 center;
			};

			const auto block = UiTransformBlock {
				math::vec2 { -1.F },
				math::vec2 { 2.F / scope_.width, 2.F / scope_.height }
			};

			cmd_.vkCommandBuffer().pushConstants(
				reinterpret_cast<VkPipelineLayout>(
					Cast<accel::VkGraphicsPipeline>(_alphaSubPass.compiled.pipeline.get())->_vkPipeLayout
				),
				vk::ShaderStageFlagBits::eVertex,
				0uL,
				sizeof(UiTransformBlock),
				&block
			);

			/**/

			auto* vb = Cast<gfx::Buffer>(_uiVertexBuffer.get());
			auto* ib = Cast<gfx::Buffer>(_uiIndexBuffer.get());

			cmd_.bindVertexBuffer(0uL, *vb, _alphaSubPass.vertexOffset);
			cmd_.bindIndexBuffer(static_cast<ref<gfx::IndexBuffer>>(*ib), _alphaSubPass.indexOffset);
		}
	);

	/* Bind default image */

	if (_defaultImage.empty()) {
		ensureDefaultImage();
	}

	const auto defaultImageGuard = _defaultImage->acquire(resource::ResourceUsageFlag::eRead);
	auto* const defaultImage = Cast<gfx::SparseTextureView>(defaultImageGuard->get());

	// UIDummy

	cmd_.bindTexture(accel::lang::SymbolId::from("ui-image"sv), defaultImage);
	cmd_.bindTextureSampler(accel::lang::SymbolId::from("ui-image-sampler"sv), _alphaSubPass.imageSampler.get());

	/**/

	if (records_._scissors.empty()) {
		return;
	}

	/* Warning: Dirty Hack for Sanitizing */

	if (records_._alpha.imageSpan.empty()) {
		const_cast<Vector<ImageSpan>&>(records_._alpha.imageSpan).emplace_back(
			records_._alpha.scissorSpan.back().last,
			records_._alpha.scissorSpan.back().last,
			~0uL
		);
	}

	/**/

	const auto& firstScissorSpan = records_._alpha.scissorSpan.front();
	const auto& lastScissorSpan = records_._alpha.scissorSpan.back();

	auto scissorIt = records_._alpha.scissorSpan.begin();
	auto imageIt = records_._alpha.imageSpan.begin();

	u32 idx = firstScissorSpan.first;
	while (idx < lastScissorSpan.last) {

		const auto startIdx = idx;
		const auto endIdx = (std::min)(
			idx < scissorIt->first ? scissorIt->first : scissorIt->last,
			idx < imageIt->first ? imageIt->first : imageIt->last
		);

		const auto& scissor = records_._scissors[scissorIt->idx];

		/**/

		const auto image = [&]() -> nmpt<gfx::TextureLikeObject> {

			if (idx < imageIt->first || idx >= imageIt->last) {
				return defaultImage;
			}

			return records_._images[imageIt->idx];
		}();

		/**/

		const auto vkScissor = vk::Rect2D {
			vk::Offset2D { static_cast<s32>(scissor.offsetX), static_cast<s32>(scissor.offsetY) },
			vk::Extent2D { static_cast<u32>(scissor.width), static_cast<u32>(scissor.height) }
		};

		cmd_.lambda(
			[vkScissor](ref<accel::AccelCommandBuffer> accel_) {
				accel_.vkCommandBuffer().setScissor(0, 1, &vkScissor);
			}
		);

		/**/

		switchType(
			image.get(),
			[](const ptr<gfx::Texture> texture_) {
				if (not texture_->vkView()) {
					gfx::TextureFactory::get()->buildView(*texture_);
				}
			},
			[](const ptr<gfx::TextureView> view_) {
				if (not view_->owner()->vkView()) {
					gfx::TextureFactory::get()->buildView(*view_->owner());
				}
			},
			[](const ptr<gfx::SparseTexture> texture_) {
				if (not texture_->_vkImageView) {
					gfx::TextureFactory::get()->buildView(*texture_);
				}
			},
			[](const ptr<gfx::SparseTextureView> view_) {
				if (not view_->vkImageView()) {
					gfx::TextureFactory::get()->buildView(*view_);
				}
			}
		);

		cmd_.bindTexture(
			accel::lang::SymbolId::from("ui-image"sv),
			image.get()
		);

		/**/

		cmd_.drawStaticMeshIdx(1uL, 0uL, (endIdx - startIdx) / 3uL, startIdx / 3uL);

		/**/

		if (scissorIt->last == endIdx) {
			++scissorIt;
		}

		if (imageIt->last == endIdx) {
			++imageIt;
		}

		idx = endIdx;
	}
}

/**/

engine::accel::EffectCompileResult build_test_opaque_pipeline(mref<smr<const engine::accel::GraphicsPass>> pass_) {

	auto effect = makeUiBaseEffect();

	auto profile = make_smr<engine::accel::EffectProfile>(
		engine::accel::EffectProfile {
			._name = "Test-UI-Profile",
			._definitions = {}
		}
	);

	/**/

	auto spec = make_smr<engine::accel::SimpleEffectSpecification>();
	spec->setPassSpec(
		make_uptr<engine::accel::GraphicsPassSpecification>(
			engine::accel::GraphicsPassSpecification {
				.depthCompareOp = engine::accel::DepthCompareOp::eGreaterEqual,
				.stencilCompareOp = engine::accel::StencilCompareOp::eNever,
				.stencilFailOp = engine::accel::StencilOp::eKeep,
				.stencilPassOp = engine::accel::StencilOp::eKeep,
				.stencilDepthFailOp = engine::accel::StencilOp::eKeep,
				.stencilCompareMask = 0uL,
				.stencilWriteMask = 0uL,
				.primitiveTopology = engine::accel::PrimitiveTopology::eTriangleList,
				.faceCulling = engine::accel::FaceCulling::eFront,
				.faceMode = engine::accel::FaceMode::eFill,
				.faceWinding = engine::accel::FaceWinding::eCcw,
				.pass = pass_.get(),
				.blendState = {
					engine::accel::BlendState {
						.defaulted = false,
						.vkState = {
							VK_FALSE,
							vk::BlendFactor::eOne,
							vk::BlendFactor::eZero,
							vk::BlendOp::eAdd,
							vk::BlendFactor::eOne,
							vk::BlendFactor::eZero,
							vk::BlendOp::eAdd,
							vk::ColorComponentFlagBits::eR |
							vk::ColorComponentFlagBits::eG |
							vk::ColorComponentFlagBits::eB |
							vk::ColorComponentFlagBits::eA
						}
					}
				}
			}
		)
	);

	/**/

	const auto compiler = engine::accel::makeVkAccCompiler();
	return compiler->compile(
		{
			.effect = std::move(effect),
			.profile = std::move(profile),
			.spec = std::move(spec)
		}
	);
}

engine::accel::EffectCompileResult build_test_alpha_pipeline(mref<smr<const engine::accel::GraphicsPass>> pass_) {

	auto effect = makeUiBaseEffect();

	auto profile = make_smr<engine::accel::EffectProfile>(
		engine::accel::EffectProfile {
			._name = "Test-UI-Profile",
			._definitions = {}
		}
	);

	/**/

	auto spec = make_smr<engine::accel::SimpleEffectSpecification>();
	spec->setPassSpec(
		make_uptr<engine::accel::GraphicsPassSpecification>(
			engine::accel::GraphicsPassSpecification {
				.depthCompareOp = engine::accel::DepthCompareOp::eGreaterEqual,
				.stencilCompareOp = engine::accel::StencilCompareOp::eNever,
				.stencilFailOp = engine::accel::StencilOp::eKeep,
				.stencilPassOp = engine::accel::StencilOp::eKeep,
				.stencilDepthFailOp = engine::accel::StencilOp::eKeep,
				.stencilCompareMask = 0uL,
				.stencilWriteMask = 0uL,
				.primitiveTopology = engine::accel::PrimitiveTopology::eTriangleList,
				.faceCulling = engine::accel::FaceCulling::eFront,
				.faceMode = engine::accel::FaceMode::eFill,
				.faceWinding = engine::accel::FaceWinding::eCcw,
				.pass = pass_.get(),
				.blendState = {
					engine::accel::BlendState {
						.defaulted = false,
						.vkState = {
							VK_TRUE,
							vk::BlendFactor::eSrcAlpha,
							vk::BlendFactor::eOneMinusSrcAlpha,
							vk::BlendOp::eAdd,
							vk::BlendFactor::eZero,
							vk::BlendFactor::eZero,
							vk::BlendOp::eAdd,
							vk::ColorComponentFlagBits::eR |
							vk::ColorComponentFlagBits::eG |
							vk::ColorComponentFlagBits::eB |
							vk::ColorComponentFlagBits::eA
						}
					}
				}
			}
		)
	);

	/**/

	const auto compiler = engine::accel::makeVkAccCompiler();
	return compiler->compile(
		{
			.effect = std::move(effect),
			.profile = std::move(profile),
			.spec = std::move(spec)
		}
	);
}

smr<const graph::Description> makeLocalDepthDescription() {
	static auto obj = make_smr<graph::TextureDescription>(
		graph::DescriptionValue { graph::DescriptionValueMatchingMode::eCovariant, TextureType::e2d },
		graph::DescriptionValue { graph::DescriptionValueMatchingMode::eInvariant, TextureFormat::eD16Unorm },
		graph::DescriptionValue { graph::DescriptionValueMatchingMode::eInvariant, 1u },
		graph::DescriptionValue { graph::DescriptionValueMatchingMode::eIgnored, graph::ActiveMipBitMask {} }
	);
	return clone(obj).into<const graph::Description>();
}

smr<graph::Symbol> makeLocalDepthSymbol() {
	return make_smr<graph::Symbol>(
		graph::SymbolFlagBits::eUndefined,
		graph::SymbolScope {
			.type = graph::SymbolScopeType::eLocal,
			.layer = ""
		},
		"local::depth",
		makeLocalDepthDescription()
	);
}

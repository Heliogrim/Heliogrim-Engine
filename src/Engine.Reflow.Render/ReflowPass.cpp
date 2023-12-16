#include "ReflowPass.hpp"

#include <cassert>
#include <Engine.Accel.Command/CommandBuffer.hpp>
#include <Engine.Accel.Compile/VkEffectCompiler.hpp>
#include <Engine.Accel.Compile/Profile/EffectProfile.hpp>
#include <Engine.Accel.Compile/Spec/SimpleEffectSpecification.hpp>
#include <Engine.Accel.Pass/VkAccelerationPassFactory.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX/Texture/Texture.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.RenderGraph/Symbol/ScopedSymbolContext.hpp>
#include <Engine.GFX/Buffer/Buffer.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBuffer.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Reflow/FlowContext.hpp>
#include <Engine.Reflow/Command/ReflowCommandBuffer.hpp>
#include <Engine.Reflow/Scene/UISceneModel.hpp>
#include <Engine.Reflow/Window/Window.hpp>
#include <Engine.Accel.Pipeline/GraphicsPipeline.hpp>
#include <Engine.Accel.Pipeline/VkGraphicsPipeline.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/AssetFactory.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets/Types/Texture/TextureAsset.hpp>
#include <Engine.Driver.Vulkan/VkRCmdTranslator.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.GFX/Texture/VirtualTextureView.hpp>
#include <Engine.GFX.Render.Predefined/Effects/UIBase.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneView.hpp>
#include <Engine.GFX.Scene/View/SceneView.hpp>
#include <Engine.Reflect/ExactType.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Scene/IRenderScene.hpp>
#include <Engine.Scene/Graph/SceneGraph.hpp>
#include <Heliogrim.Default/Assets/Textures/UIDummy.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.Assets/Types/Texture/TextureAsset.hpp>
#include <Engine.GFX.Loader/Texture/TextureLoader.hpp>
#include <Engine.GFX/Texture/TextureFactory.hpp>
#include <Engine.GFX/Texture/TextureView.hpp>
#include <Engine.Reflect/TypeSwitch.hpp>
#include <Heliogrim.Default/Assets/Images/UIDummy.hpp>

using namespace hg::engine::reflow;
using namespace hg::engine::render;
using namespace hg;

[[nodiscard]] static engine::accel::EffectCompileResult build_test_base_pipeline(
    mref<smr<const engine::accel::GraphicsPass>> pass_
);

/**/

void render::ReflowPass::destroy() noexcept {
    SubPass::destroy();

    const auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();
    device->vkDevice().destroySemaphore(_STD exchange(_tmpSignal, nullptr));

    if (_framebuffer) {
        _framebuffer->destroy();
        _framebuffer.reset();
    }

    if (_uiVertexBuffer) {
        Cast<gfx::Buffer>(_uiVertexBuffer.get())->destroy();
        _uiVertexBuffer.reset();
    }

    if (_uiIndexBuffer) {
        Cast<gfx::Buffer>(_uiIndexBuffer.get())->destroy();
        _uiIndexBuffer.reset();
    }

    if (_uiImageSampler) {
        _uiImageSampler->destroy();
        _uiImageSampler.reset();
    }

    if (_uiMsdfSampler) {
        _uiMsdfSampler->destroy();
        _uiMsdfSampler.reset();
    }

    _uiBaseCompiled.pipeline.reset();
    _uiMsdfCompiled.pipeline.reset();

    _graphicsPass.reset();
}

void render::ReflowPass::iterate(cref<engine::render::graph::ScopedSymbolContext> symCtx_) noexcept {

    ensureGraphicsPass();

}

void render::ReflowPass::resolve() noexcept {

    if (_uiBaseCompiled.flag == accel::EffectCompileResultFlag::eUnknown) {
        _uiBaseCompiled = build_test_base_pipeline(clone(_graphicsPass));
    }

    if (_uiMsdfCompiled.flag == accel::EffectCompileResultFlag::eUnknown) {
        //_uiMsdfCompiled = build_test_msdf_pipeline(clone(_graphicsPass));
    }

    if (_uiImageSampler == nullptr) {
        _uiImageSampler = make_uptr<gfx::TextureSampler>();
        _uiImageSampler->setup(Engine::getEngine()->getGraphics()->getCurrentDevice());
    }

    if (_uiMsdfSampler == nullptr) {}
}

void render::ReflowPass::execute(cref<engine::render::graph::ScopedSymbolContext> symCtx_) noexcept {

    const auto sceneColor = symCtx_.getExportSymbol(makeSceneColorSymbol());
    const auto sceneColorRes = sceneColor->load<smr<gfx::TextureLikeObject>>();
    const auto sceneColorTex = Cast<gfx::Texture>(sceneColorRes.get());

    /**/

    assert(not sceneColor->empty());
    ensureFramebuffer(clone(sceneColorRes));

    /**/

    const auto* uiModel = static_cast<const ptr<const UISceneModel>>(nullptr);

    {
        auto sceneViewRes = symCtx_.getImportSymbol(makeSceneViewSymbol());
        auto sceneView = sceneViewRes->load<smr<const gfx::scene::SceneView>>();

        auto scene = sceneView->getScene()->renderGraph();
        scene->traversal(
            [&uiModel](const ptr<scene::SceneGraph<gfx::SceneNodeModel>::node_type> node_) -> bool {

                auto size = node_->exclusiveSize();
                for (decltype(size) i = 0; i < size; ++i) {

                    auto el = node_->elements()[i];
                    if (ExactType<UISceneModel>(*el)) {
                        uiModel = static_cast<UISceneModel*>(el);
                        return false;
                    }

                }

                return uiModel == nullptr;
            }
        );
    }

    if (uiModel == nullptr) {
        return;
    }

    /**/

    ReflowCommandBuffer uiCmd {};
    uiCmd._runningIndexes.reserve(32uLL * 1024uLL);
    uiCmd._runningVertices.reserve(32uLL * 1024uLL);

    math::vec2 available {
        static_cast<float>(sceneColorTex->width()),
        static_cast<float>(sceneColorTex->height())
    };
    const FlowContext fctx {
        math::fExtent2D { available.x, available.y, 0.F, 0.F },
        math::fExtent2D { available.x, available.y, 0.F, 0.F }
    };

    const math::fExtent2D rootScissor { fctx.scissor };
    uiCmd.pushScissor(rootScissor);

    /**/

    auto& wnd = uiModel->getWindow();
    wnd.render(&uiCmd);

    assert(rootScissor == uiCmd.popScissor());

    /**/

    const auto& uiRv = uiCmd._runningVertices;
    const auto& uiRi = uiCmd._runningIndexes;

    /**/

    if (uiRv.empty() || uiRi.empty()) {
        return;
    }

    updateVertices(uiRv);
    updateIndices(uiRi);

    /**/

    cmd::RenderCommandBuffer cmd {};

    cmd.begin();
    cmd.beginAccelPass({ .pass = _graphicsPass.get(), .framebuffer = _framebuffer.get() });
    cmd.beginSubPass();

    cmd.bindGraphicsPipeline(clone(_uiBaseCompiled.pipeline).into<accel::GraphicsPipeline>());

    cmd.lambda(
        [this, available](ref<accel::AccelCommandBuffer> cmd_) {

            struct UiTransformBlock {
                math::vec2 extent;
                math::vec2 center;
            };

            const auto block = UiTransformBlock {
                math::vec2 { -1.F },
                math::vec2 { 2.F / available.x, 2.F / available.y }
            };

            cmd_.vkCommandBuffer().pushConstants(
                reinterpret_cast<VkPipelineLayout>(
                    Cast<accel::VkGraphicsPipeline>(_uiBaseCompiled.pipeline.get())->_vkPipeLayout
                ),
                vk::ShaderStageFlagBits::eVertex,
                0uL,
                sizeof(UiTransformBlock),
                &block
            );

            auto* vb = Cast<gfx::Buffer>(_uiVertexBuffer.get());
            auto* ib = Cast<gfx::Buffer>(_uiIndexBuffer.get());

            cmd_.bindVertexBuffer(0uL, *vb, 0uLL);
            cmd_.bindIndexBuffer(static_cast<ref<gfx::IndexBuffer>>(*ib), 0uLL);
        }
    );

    /* Bind default image */

    if (_defaultImage.empty()) {
        ensureDefaultImage();
    }

    const auto defaultImageGuard = _defaultImage->acquire(resource::ResourceUsageFlag::eRead);
    const auto* const defaultImage = Cast<gfx::VirtualTextureView>(defaultImageGuard->get());

    // UIDummy

    cmd.bindTexture(
        accel::lang::SymbolId::from("ui-image"),
        defaultImage,
        _uiImageSampler.get()
    );

    /**/

    {

        u32 firstIndices = static_cast<u32>(uiRi.size());
        if (not uiCmd._scissorIndices.empty() && uiCmd._scissorIndices.front().first < firstIndices) {
            firstIndices = uiCmd._scissorIndices.front().first;
        }

        if (not uiCmd._imageIndices.empty() && uiCmd._imageIndices.front().first < firstIndices) {
            firstIndices = uiCmd._imageIndices.front().first;
        }

        /**/

        u32 lastStartIndices = static_cast<u32>(uiRi.size());
        if (not uiCmd._scissorIndices.empty() && uiCmd._scissorIndices.back().second < lastStartIndices) {
            lastStartIndices = uiCmd._scissorIndices.back().second;
        }

        if (not uiCmd._imageIndices.empty() && uiCmd._imageIndices.back().second < lastStartIndices) {
            lastStartIndices = uiCmd._imageIndices.back().second;
        }

        /**/

        if (uiCmd._scissorIndices.empty()) {
            uiCmd._scissorIndices.emplace_back(~0uL, ~0uL);
        }

        if (uiCmd._imageIndices.empty()) {
            uiCmd._imageIndices.emplace_back(~0uL, ~0uL);
        }

        /**/

        auto sciIdxIt = 0uL;
        auto imgIdxIt = 0uL;

        vk::Rect2D vkScissor {
            vk::Offset2D { static_cast<s32>(rootScissor.offsetX), static_cast<s32>(rootScissor.offsetY) },
            vk::Extent2D { static_cast<u32>(rootScissor.width), static_cast<u32>(rootScissor.height) }
        };

        /**/

        cmd.lambda(
            [vkScissor, firstIndices](ref<accel::AccelCommandBuffer> cmd_) {
                cmd_.vkCommandBuffer().setScissor(0, 1, &vkScissor);
            }
        );
        cmd.drawStaticMeshIdx(1uL, 0uL, firstIndices / 3uL, 0uL);

        /**/

        u32 idx = firstIndices;
        while (idx < lastStartIndices) {

            const auto sciIdx = idx >= uiCmd._scissorIndices[sciIdxIt].second ? ++sciIdxIt : sciIdxIt;
            const auto imgIdx = idx >= uiCmd._imageIndices[imgIdxIt].second ? ++imgIdxIt : imgIdxIt;

            auto nextStride = MIN(uiCmd._scissorIndices[sciIdx].second, uiCmd._imageIndices[imgIdxIt].second);

            if (uiCmd._scissorIndices[sciIdx].first > idx && uiCmd._scissorIndices[sciIdx].first < nextStride) {
                nextStride = uiCmd._scissorIndices[sciIdx].first;
            }

            if (uiCmd._imageIndices[imgIdx].first > idx && uiCmd._imageIndices[imgIdx].first < nextStride) {
                nextStride = uiCmd._imageIndices[imgIdx].first;
            }

            /**/

            const auto& recs = uiCmd._scissors[sciIdx];

            vkScissor.offset.x = static_cast<s32>(recs.offsetX);
            vkScissor.offset.y = static_cast<s32>(recs.offsetY);
            vkScissor.extent.width = static_cast<u32>(recs.width);
            vkScissor.extent.height = static_cast<u32>(recs.height);

            cmd.lambda(
                [vkScissor](ref<accel::AccelCommandBuffer> cmd_) {
                    cmd_.vkCommandBuffer().setScissor(0, 1, &vkScissor);
                }
            );

            if (uiCmd._imageIndices[imgIdxIt].first <= idx && idx < uiCmd._imageIndices[imgIdxIt].second) {

                /* Warning: Temporary Solution */
                switchType(
                    uiCmd._images[imgIdx].get(),
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
                    [](const ptr<gfx::VirtualTexture> texture_) {
                        if (not texture_->_vkImageView) {
                            gfx::TextureFactory::get()->buildView(*texture_);
                        }
                    },
                    [](const ptr<gfx::VirtualTextureView> view_) {
                        if (not view_->vkImageView()) {
                            gfx::TextureFactory::get()->buildView(*view_);
                        }
                    }
                );

                /**/

                cmd.bindTexture(
                    accel::lang::SymbolId::from("ui-image"),
                    uiCmd._images[imgIdx].get(),
                    _uiImageSampler.get()
                );

            } else {

                /* Not in scope -> Bind default image */
                cmd.bindTexture(
                    accel::lang::SymbolId::from("ui-image"),
                    defaultImage,
                    _uiImageSampler.get()
                );
            }

            cmd.drawStaticMeshIdx(1uL, 0uL, (nextStride - idx) / 3uL, idx / 3uL);

            /**/

            idx = nextStride;
        }

        /**/

        if (lastStartIndices < uiCmd._runningIndexes.size()) {

            vkScissor.offset.x = static_cast<s32>(rootScissor.offsetX);
            vkScissor.offset.y = static_cast<s32>(rootScissor.offsetY);
            vkScissor.extent.width = static_cast<u32>(rootScissor.width);
            vkScissor.extent.height = static_cast<u32>(rootScissor.height);

            /* Rebind default image for tail stride */
            cmd.bindTexture(
                accel::lang::SymbolId::from("ui-image"),
                defaultImage,
                _uiImageSampler.get()
            );

            cmd.lambda(
                [vkScissor](ref<accel::AccelCommandBuffer> cmd_) {
                    cmd_.vkCommandBuffer().setScissor(0, 1, &vkScissor);
                }
            );

            cmd.drawStaticMeshIdx(1uL, 0uL, (uiRi.size() - lastStartIndices) / 3uL, lastStartIndices / 3uL);
        }
    }

    /**/

    cmd.endSubPass();
    cmd.endAccelPass();
    cmd.end();

    /**/

    const auto translator = make_uptr<driver::vk::VkRCmdTranslator>();
    auto batch = (*translator)(&cmd);

    {
        batch->_tmpWaits.insert_range(
            batch->_tmpWaits.end(),
            reinterpret_cast<Vector<VkSemaphore>&>(sceneColor->barriers)
        );
        batch->_tmpWaits.insert_range(
            batch->_tmpWaits.end(),
            reinterpret_cast<Vector<VkSemaphore>&>(uiCmd._imageWait)
        );

        for (auto i = batch->_tmpWaitFlags.size(); i < batch->_tmpWaits.size(); ++i) {
            batch->_tmpWaitFlags.emplace_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        }

        batch->_tmpSignals.emplace_back(_tmpSignal);
        batch->_tmpSignals.insert_range(
            batch->_tmpSignals.end(),
            reinterpret_cast<Vector<VkSemaphore>&>(uiCmd._imageSignal)
        );

        /**/

        sceneColor->barriers.clear();
        sceneColor->barriers.emplace_back(_tmpSignal.operator VkSemaphore());
    }

    batch->commitAndDispose();
    delete batch;
}

void render::ReflowPass::ensureDefaultImage() {

    const auto* const factory = Engine::getEngine()->getAssets()->getFactory();
    auto* const registry = Engine::getEngine()->getAssets()->getRegistry();

    if (not registry->hasAsset(game::assets::image::UIDummy::unstable_auto_guid())) {
        factory->createImageAsset(
            game::assets::image::UIDummy::unstable_auto_guid(),
            R"(resources\imports\ktx\default_ui.ktx)"
        );
    }

    if (not registry->hasAsset(game::assets::texture::UIDummy::unstable_auto_guid())) {
        delete(new(game::assets::texture::UIDummy));
    }

    /**/

    auto* const asset = registry->findAssetByGuid(game::assets::texture::UIDummy::unstable_auto_guid());
    assert(asset != nullptr);

    /**/

    auto request = static_cast<ptr<assets::TextureAsset>>(asset);
    auto resource = Engine::getEngine()->getResources()->loader().load<assets::TextureAsset, gfx::TextureResource>(
        _STD move(request)
    );

    auto guard = resource->acquire(resource::ResourceUsageFlag::eAll);
    auto* const image = Cast<gfx::VirtualTextureView>(guard->get());
    assert(image);

    if (not image->vkImageView()) {
        engine::gfx::TextureFactory::get()->buildView(*image, { .type = TextureType::e2d });
    }

    _defaultImage = _STD move(resource);
}

void render::ReflowPass::ensureGraphicsPass() {

    if (not _graphicsPass.empty()) {
        return;
    }

    constexpr auto factory = accel::VkAccelerationPassFactory();
    _graphicsPass = factory.buildGraphicsPass(
        {
            makeSceneColorSymbol()
        },
        {
            makeSceneColorSymbol()
        }
    ).value();
}

void render::ReflowPass::ensureFramebuffer(mref<smr<gfx::TextureLikeObject>> colorTarget_) {

    if (_framebuffer != nullptr && _framebuffer->attachments().front() != colorTarget_) {
        _framebuffer->device()->vkDevice().destroySemaphore(_tmpSignal);
        _framebuffer->destroy();
        _framebuffer.reset();
    }

    if (_framebuffer != nullptr) {
        return;
    }

    _framebuffer = make_uptr<gfx::Framebuffer>(Engine::getEngine()->getGraphics()->getCurrentDevice());

    const auto* const texture = Cast<gfx::Texture>(colorTarget_.get());
    _framebuffer->addAttachment(clone(colorTarget_));

    _framebuffer->setExtent(texture->extent());
    _framebuffer->setRenderPass(clone(_graphicsPass));

    _framebuffer->setup();
    _tmpSignal = _framebuffer->device()->vkDevice().createSemaphore({});
}

void render::ReflowPass::updateVertices(cref<Vector<gfx::uivertex>> vertices_) {

    const auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();
    const auto vertexSize = sizeof(gfx::uivertex) * vertices_.size();

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
            device->allocator(),
            device,
            next.buffer,
            gfx::MemoryProperty::eHostVisible,
            next.memory
        );
        next.bind();
    }

    /**/

    Cast<gfx::Buffer>(_uiVertexBuffer.get())->write<gfx::uivertex>(
        vertices_.data(),
        vertices_.size()
    );
}

void render::ReflowPass::updateIndices(cref<Vector<u32>> indices_) {

    const auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();
    const auto indexSize = sizeof(u32) * indices_.size();

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
            device->allocator(),
            device,
            next.buffer,
            gfx::MemoryProperty::eHostVisible,
            next.memory
        );
        next.bind();
    }

    /**/

    Cast<gfx::Buffer>(_uiIndexBuffer.get())->write<u32>(
        indices_.data(),
        indices_.size()
    );
}

/**/

engine::accel::EffectCompileResult build_test_base_pipeline(mref<smr<const engine::accel::GraphicsPass>> pass_) {

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
                .depthCompareOp = engine::accel::DepthCompareOp::eNever,
                .stencilCompareOp = engine::accel::StencilCompareOp::eNever,
                .stencilFailOp = engine::accel::StencilOp::eKeep,
                .stencilPassOp = engine::accel::StencilOp::eKeep,
                .stencilDepthFailOp = engine::accel::StencilOp::eKeep,
                .stencilCompareMask = 0uL,
                .stencilWriteMask = 0uL,
                .primitiveTopology = engine::accel::PrimitiveTopology::eTriangleList,
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
            .effect = _STD move(effect),
            .profile = _STD move(profile),
            .spec = _STD move(spec)
        }
    );
}

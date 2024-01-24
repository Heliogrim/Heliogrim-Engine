#include "DummyProvider.hpp"

#include <Engine.Accel.Command/CommandBuffer.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Driver.Vulkan/VkRCmdTranslator.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneDepth.hpp>
#include <Engine.GFX.RenderGraph/Relation/TextureDescription.hpp>
#include <Engine.GFX.RenderGraph/Symbol/ScopedSymbolContext.hpp>
#include <Engine.GFX/Texture/TextureFactory.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBuffer.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/Cast.hpp>

using namespace hg::engine::render;
using namespace hg::engine::gfx;
using namespace hg;

void DummyProvider::destroy() noexcept {
    SubPass::destroy();

    auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();
    device->vkDevice().destroySemaphore(static_cast<VkSemaphore>(std::exchange(_tmpSignal, nullptr)));

    _resources.outDepthTexture->destroy<smr<TextureLikeObject>, graph::TextureDescription>();

    _depthTexture->destroy();
    _depthTexture.reset();
}

void DummyProvider::declareOutputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept {
    symCtx_.registerExposeSymbol(makeSceneDepthSymbol(), &_resources.outDepthTexture);
}

void DummyProvider::iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept {}

void DummyProvider::resolve() noexcept {}

void DummyProvider::execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

    auto sceneColor = symCtx_.getExportSymbol(makeSceneColorSymbol());
    const auto& sceneColorTex = sceneColor->load<smr<const TextureLikeObject>, graph::TextureDescription>();

    assert(not sceneColor->empty() && not sceneColorTex.empty());

    if (_tmpSignal == nullptr) {
        const auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();
        _tmpSignal = device->vkDevice().createSemaphore({});
    }

    cmd::RenderCommandBuffer cmd {};
    cmd.begin();
    cmd.lambda(
        [sceneColorTex](ref<accel::AccelCommandBuffer> cmd_) {

            const auto* const texture = Cast<gfx::Texture>(sceneColorTex.get());

            const auto barrier = vk::ImageMemoryBarrier {
                vk::AccessFlags(),
                vk::AccessFlags(),
                vk::ImageLayout::eUndefined,
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
            };

            cmd_.vkCommandBuffer().pipelineBarrier(
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::PipelineStageFlagBits::eTopOfPipe,
                vk::DependencyFlags(),
                0uL,
                nullptr,
                0uL,
                nullptr,
                1uL,
                &barrier
            );
        }
    );
    cmd.end();

    /**/

    auto translator = make_uptr<driver::vk::VkRCmdTranslator>();
    auto nativeBatch = (*translator)(&cmd);
    const auto batch = static_cast<ptr<driver::vk::VkNativeBatch>>(nativeBatch.get());

    {
        batch->_tmpWaits.insert_range(
            batch->_tmpWaits.end(),
            reinterpret_cast<Vector<VkSemaphore>&>(sceneColor->barriers)
        );
        for (auto i = batch->_tmpWaitFlags.size(); i < batch->_tmpWaits.size(); ++i) {
            batch->_tmpWaitFlags.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        }
        batch->_tmpSignals.push_back(reinterpret_cast<VkSemaphore>(_tmpSignal));

        sceneColor->barriers.clear();
        sceneColor->barriers.push_back(reinterpret_cast<VkSemaphore>(_tmpSignal));
    }

    batch->commitAndDispose();
    nativeBatch.reset();

    /**/

    auto sct = clone(sceneColorTex).into<Texture>();
    if (not _depthTexture.empty() && _depthTexture->extent() != sct->extent()) {
        _depthTexture->destroy();
        _depthTexture.reset();
    }

    if (_depthTexture.empty()) {

        const auto* const factory = TextureFactory::get();
        auto sceneDepthTex = factory->build(
            {
                sct->extent(), TextureFormat::eD32Sfloat, 1uL, TextureType::e2d, vk::ImageAspectFlagBits::eDepth,
                vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
                vk::MemoryPropertyFlagBits::eDeviceLocal,
                vk::SharingMode::eExclusive
            }
        );
        factory->buildView(sceneDepthTex, { .layers = { 0L, 1L }, .type = TextureType::e2d, .mipLevels = { 0L, 1L } });

        _depthTexture = make_smr<Texture>(std::move(sceneDepthTex));

        if (_resources.outDepthTexture->empty()) {

            _resources.outDepthTexture.obj()->symbol = makeSceneDepthSymbol();
            _resources.outDepthTexture.obj()->owned = true;

            _resources.outDepthTexture->create<
                smr<TextureLikeObject>,
                graph::TextureDescription,
                decltype([](const auto& obj_) {
                    return obj_.get();
                })
            >(clone(_depthTexture).into<TextureLikeObject>());
        } else {
            _resources.outDepthTexture->store<
                smr<TextureLikeObject>,
                graph::TextureDescription,
                decltype([](const auto& obj_) {
                    return obj_.get();
                })
            >(clone(_depthTexture).into<TextureLikeObject>());
        }

        /**/
        const_cast<ref<graph::ScopedSymbolContext>>(symCtx_).eraseExposedSymbol(makeSceneDepthSymbol());
        const_cast<ref<graph::ScopedSymbolContext>>(symCtx_).exposeSymbol(
            makeSceneDepthSymbol(),
            _resources.outDepthTexture.obj()
        );
    }
}

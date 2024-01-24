#include "TmpEndPass.hpp"

#include <Engine.Accel.Command/CommandBuffer.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Driver.Vulkan/VkRCmdTranslator.hpp>
#include <Engine.GFX.Render.Command/RenderCommandBuffer.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneColor.hpp>
#include <Engine.GFX.Render.Predefined/Symbols/SceneDepth.hpp>
#include <Engine.GFX.RenderGraph/Symbol/ScopedSymbolContext.hpp>
#include <Engine.GFX/Texture/Texture.hpp>
#include <Engine.Reflect/Cast.hpp>

using namespace hg::engine::render;
using namespace hg;

TmpEndPass::TmpEndPass(const vk::ImageLayout targetLayout_) :
    SubPass(),
    _targetLayout(targetLayout_),
    _tmpSignal() {}

void TmpEndPass::destroy() noexcept {
    SubPass::destroy();

    auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();
    device->vkDevice().destroySemaphore(static_cast<VkSemaphore>(std::exchange(_tmpSignal, nullptr)));
}

void TmpEndPass::declareTransforms(ref<graph::ScopedSymbolContext> symCtx_) noexcept {
    SubPass::declareTransforms(symCtx_);

    symCtx_.registerImportSymbol(makeSceneColorSymbol(), &_resources.inOutSceneColor);
    symCtx_.registerExportSymbol(makeSceneColorSymbol(), &_resources.inOutSceneColor);
}

void TmpEndPass::iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept {}

void TmpEndPass::resolve() noexcept {}

void TmpEndPass::execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept {

    if (_tmpSignal == nullptr) {
        const auto device = Engine::getEngine()->getGraphics()->getCurrentDevice();
        _tmpSignal = device->vkDevice().createSemaphore({});
    }

    auto sceneColorRes = symCtx_.getExportSymbol(makeSceneColorSymbol());
    const auto& sceneColorTex = sceneColorRes->load<smr<const gfx::TextureLikeObject>>();

    cmd::RenderCommandBuffer cmd {};
    cmd.begin();
    cmd.lambda(
        [sceneColorTex, this](ref<accel::AccelCommandBuffer> cmd_) {

            const auto* const texture = Cast<gfx::Texture>(sceneColorTex.get());

            const auto barrier = vk::ImageMemoryBarrier {
                vk::AccessFlags(),
                vk::AccessFlags(),
                vk::ImageLayout::eColorAttachmentOptimal,
                _targetLayout,
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
            reinterpret_cast<Vector<VkSemaphore>&>(sceneColorRes->barriers)
        );
        for (auto i = batch->_tmpWaitFlags.size(); i < batch->_tmpWaits.size(); ++i) {
            batch->_tmpWaitFlags.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
        }
        batch->_tmpSignals.push_back(reinterpret_cast<VkSemaphore>(_tmpSignal));

        sceneColorRes->barriers.clear();
        sceneColorRes->barriers.push_back(reinterpret_cast<VkSemaphore>(_tmpSignal));
    }

    batch->commitAndDispose();
    nativeBatch.reset();
}

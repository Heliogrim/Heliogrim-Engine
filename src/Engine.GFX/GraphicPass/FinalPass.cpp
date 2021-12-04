#include "FinalPass.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "FinalPassCompositeStage.hpp"
#include "../Graphics.hpp"
#include "../Texture/TextureFactory.hpp"

using namespace ember::engine::gfx;
using namespace ember;

FinalPass::FinalPass(cref<sptr<Device>> device_, const ptr<Swapchain> swapchain_) :
    GraphicPass(device_, swapchain_, GraphicPassMask::eFinalPass) {}

void FinalPass::setup() {

    SCOPED_STOPWATCH

    auto finalStage = new FinalPassCompositeStage { this };
    finalStage->setup();
    _pipeline.add(finalStage);

    const auto factory = TextureFactory::get();

    _framebuffers.clear();
    _framebuffers.reserve(_swapchain->length());

    for (u32 i = 0; i < _swapchain->length(); ++i) {
        /**
         *
         */
        Framebuffer buffer { _device };

        buffer.setExtent(math::uivec3 { _swapchain->extent(), 1ui32 });
        buffer.setRenderPass(finalStage->renderPass());

        /**
         *
         */
        cref<sptr<Texture>> color = _swapchain->at(i);

        /**
         *
         */
        buffer.add(FramebufferAttachment { color });

        /**
         *
         */
        buffer.setup();
        _framebuffers.push_back(_STD move(buffer));
    }
}

void FinalPass::destroy() {

    for (auto& entry : _framebuffers) {

        for (auto& attachment : entry.attachments()) {
            attachment->destroy();
        }

        entry.destroy();
    }

    _framebuffers.clear();

    auto& stages = _pipeline.stages();
    for (auto& entry : stages) {
        delete entry;
        entry = nullptr;
    }
}

void FinalPass::process(const ptr<scene::RenderGraph> graph_, ref<CommandBatch> batch_) {

    SCOPED_STOPWATCH

    _pipeline.process(nullptr, batch_);
}

ptr<ModelPassProcessor> FinalPass::processor() noexcept {
    return nullptr;
}

cref<Framebuffer> FinalPass::framebuffer(u32 idx_) const {
    return _framebuffers[idx_];
}

cref<Framebuffer> FinalPass::currentFramebuffer() const noexcept {
    return _framebuffers[_swapchain->currentIdx()];
}

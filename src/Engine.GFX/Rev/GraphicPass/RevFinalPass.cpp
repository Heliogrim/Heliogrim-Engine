#include "RevFinalPass.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "RevFinalPassCompositeStage.hpp"
#include "../../Graphics.hpp"
#include "../../Texture/TextureFactory.hpp"

#include "RevDepthPass.hpp"

using namespace ember::engine::gfx;
using namespace ember;

RevFinalPass::RevFinalPass(cref<sptr<Device>> device_) :
    GraphicPass(device_, GraphicPassMask::eFinalPass) {}

void RevFinalPass::setup() {

    SCOPED_STOPWATCH

    auto finalStage = new RevFinalPassCompositeStage { this };
    finalStage->setup();
    _pipeline.add(finalStage);
}

void RevFinalPass::destroy() {
    auto& stages = _pipeline.stages();
    for (auto it { stages.rbegin() }; it != stages.rend(); ++it) {

        auto& stage { *it };

        stage->destroy();
        delete stage;
        stage = nullptr;
    }

    stages.clear();
}

void RevFinalPass::allocateWith(const ptr<const RenderInvocation> invocation_,
    const ptr<RenderInvocationState> state_) {

    const auto* factory { TextureFactory::get() };
    const auto* compositeStage { static_cast<ptr<RevFinalPassCompositeStage>>(_pipeline.stages()[0]) };

    Framebuffer buffer { _device };
    buffer.setExtent(invocation_->target()->extent());
    buffer.setRenderPass(compositeStage->renderPass());

    /**
     *
     */
    auto& texture { *invocation_->target() };

    if (!texture.vkView()) {
        factory->buildView(texture);
    }

    /**
     *
     */
    buffer.add({ _STD move(texture) });

    /**
     *
     */
    buffer.setup();
    state_->data.insert_or_assign("RevFinalPass::Framebuffer"sv, _STD make_shared<decltype(buffer)>(_STD move(buffer)));
    state_->framebuffer = _STD static_pointer_cast<Framebuffer, void>(
        state_->data.at("RevFinalPass::Framebuffer"sv)
    ).get();

    /**
     *
     */
    GraphicPass::allocateWith(invocation_, state_);
}

void RevFinalPass::freeWith(const ptr<const RenderInvocation> invocation_, const ptr<RenderInvocationState> state_) {
    GraphicPass::freeWith(invocation_, state_);

    /**
     *
     */
    const auto it { state_->data.find("RevFinalPass::Framebuffer"sv) };
    if (it != state_->data.end()) {

        sptr<Framebuffer> buffer {
            _STD static_pointer_cast<Framebuffer, void>(it->second)
        };

        /**
         *
         */
        buffer->destroy();

        for (auto& entry : buffer->attachments()) {
            //entry->destroy();
        }

        /**
         *
         */
        state_->data.erase(it);
    }
}

void RevFinalPass::process(const ptr<scene::RenderGraph> graph_, const ptr<const RenderContext> ctx_,
    ref<CommandBatch> batch_) {

    SCOPED_STOPWATCH

    _pipeline.process(nullptr, ctx_, batch_);
}

ptr<GraphicPassModelProcessor> RevFinalPass::processor() noexcept {
    return nullptr;
}

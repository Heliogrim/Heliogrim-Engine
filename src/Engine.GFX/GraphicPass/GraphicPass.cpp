#include "GraphicPass.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace ember::engine::gfx;
using namespace ember;

GraphicPass::GraphicPass(cref<sptr<Device>> device_, cref<GraphicPassMask> mask_) :
    _device(device_),
    _mask(mask_) {}

void GraphicPass::allocateWith(const ptr<const RenderInvocation> invocation_,
    const ptr<RenderInvocationState> state_) {
    _pipeline.allocateWith(invocation_, state_);
}

void GraphicPass::freeWith(const ptr<const RenderInvocation> invocation_, const ptr<RenderInvocationState> state_) {
    _pipeline.freeWith(invocation_, state_);
}

void GraphicPass::process(const ptr<scene::RenderGraph> graph_, const ptr<const RenderContext> ctx_,
    ref<CommandBatch> batch_) {

    SCOPED_STOPWATCH

    /**
     * Get Processor
     */
    ptr<GraphicPassModelProcessor> processor = this->processor();

    /**
     * Process given RenderGraph with processor instance
     */
    if (processor) {
        struct ConsumerWrapper {

            ref<GraphicPassModelProcessor> obj;

            bool (GraphicPassModelProcessor::*fnc)(u32, const ptr<scene::RenderGraph::node_type>);

            [[nodiscard]] bool operator()(u32 batchIdx_,
                const ptr<scene::RenderGraph::node_type> node_) const noexcept {
                return (obj.*fnc)(batchIdx_, node_);
            }
        };

        const ConsumerWrapper consumer { *processor, &GraphicPassModelProcessor::operator() };
        graph_->traversalBatched(0, consumer);

        /**
         * Post-Process the consumed data
         */
        processor->postProcess(ctx_);
    }

    /**
     *
     */
    _pipeline.process(processor, ctx_, batch_);
}

const sptr<Device> GraphicPass::device() const noexcept {
    return _device;
}

GraphicPassMask GraphicPass::mask() const noexcept {
    return _mask;
}

GraphicPassMask GraphicPass::mask() noexcept {
    return _mask;
}

cref<GraphicPassPipeline> GraphicPass::pipeline() const noexcept {
    return _pipeline;
}

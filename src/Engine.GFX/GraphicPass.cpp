#include "GraphicPass.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

using namespace ember::engine::gfx;
using namespace ember;

GraphicPass::GraphicPass(cref<sptr<Device>> device_, const ptr<Swapchain> swapchain_, cref<GraphicPassMask> mask_) :
    _device(device_),
    _swapchain(swapchain_),
    _mask(mask_) {}

void GraphicPass::process(const ptr<scene::RenderGraph> graph_, ref<CommandBatch> batch_) {

    SCOPED_STOPWATCH

    ptr<ModelPassProcessor> processor = this->processor();
    if (processor) {
        struct ConsumerWrapper {

            ref<ModelPassProcessor> obj;

            bool (ModelPassProcessor::*fnc)(u32, const ptr<scene::RenderGraph::node_type>);

            [[nodiscard]] bool operator()(u32 batchIdx_,
                const ptr<scene::RenderGraph::node_type> node_) const noexcept {
                return (obj.*fnc)(batchIdx_, node_);
            }
        };

        const ConsumerWrapper consumer { *processor, &ModelPassProcessor::operator() };
        graph_->traversalBatched(0, consumer);
    }
    _pipeline.process(processor, batch_);
}

const sptr<Device> GraphicPass::device() const noexcept {
    return _device;
}

const ptr<Swapchain> GraphicPass::swapchain() const noexcept {
    return _swapchain;
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

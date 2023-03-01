#include "GraphicPassModelProcessor.hpp"

#if FALSE

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "GraphicPass.hpp"

using namespace hg::engine::gfx;
using namespace hg;

GraphicPassModelProcessor::GraphicPassModelProcessor(ptr<const GraphicPass> graphicPass_) :
    _graphicPass(graphicPass_) {}

bool GraphicPassModelProcessor::operator()(u32 batchIdx_, const ptr<scene::RenderGraph::node_type> node_) noexcept {

    SCOPED_STOPWATCH

#if FALSE
    /**
     * TODO: Check whether node is used for current RenderPass
     */
    if (false) {
        /**
         * Return false for consumer to discard this and child element from traversal
         */
        return false;
    }

    /**
     *
     */
    //auto* elements { node_.elements() };
    auto nse = node_.payload().payload<SceneElement>().lock();
    ref<SceneElement> se = *nse;

    /**
     *
     */
    auto batch = se.batch(_graphicPass->mask());

    /**
     *
     */
    auto state = se.state();

    /**
     * Return true for consumer to traverse the child elements in tree
     */
    return true;
#endif
    return true;
}

void GraphicPassModelProcessor::process(mref<ModelBatch> model_) noexcept {
    // TODO:
}

void GraphicPassModelProcessor::postProcess(const ptr<const RenderContext> ctx_) {}

cref<Vector<ProcessedModelBatch>> GraphicPassModelProcessor::models() const noexcept {
    return _consumed;
}

ptr<const GraphicPass> GraphicPassModelProcessor::graphicPass() const noexcept {
    return _graphicPass;
}

ptr<const GraphicPass> GraphicPassModelProcessor::graphicPass() noexcept {
    return _graphicPass;
}

#endif

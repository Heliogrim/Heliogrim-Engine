#include "ModelPassProcessor.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "GraphicPass.hpp"
#include "Scene/SceneElement.hpp"

using namespace ember::engine::gfx;
using namespace ember;

ModelPassProcessor::ModelPassProcessor(ptr<const GraphicPass> graphicPass_) :
    _graphicPass(graphicPass_) {}

bool ModelPassProcessor::operator()(u32 batchIdx_, cref<scene::SceneNode> node_) noexcept {

    SCOPED_STOPWATCH

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
}

cref<std::vector<ProcessedModelBatch>> ModelPassProcessor::models() const noexcept {
    return _consumed;
}

ptr<const GraphicPass> ModelPassProcessor::graphicPass() const noexcept {
    return _graphicPass;
}

ptr<const GraphicPass> ModelPassProcessor::graphicPass() noexcept {
    return _graphicPass;
}

#include "DepthModelPassProcessor.hpp"

#ifdef _PROFILING
#include <Engine.Common/Profiling/Stopwatch.hpp>
#endif

#include "../GraphicPass.hpp"
#include "../Scene/SceneElement.hpp"

using namespace ember::engine::gfx;
using namespace ember;

DepthModelPassProcessor::DepthModelPassProcessor(ptr<const GraphicPass> graphicPass_) :
    ModelPassProcessor(graphicPass_) {}

bool DepthModelPassProcessor::operator()(cref<scene::SceneNode> node_) noexcept {

    SCOPED_STOPWATCH

    /**
     * Precheck whether node has state or children
     */
    if (node_.isLeaf()) {
        return false;
    }

    /**
     * Check whether scene node as stored valid element reference; otherwise drop iteration
     */
    auto& stored = node_.payload();
    auto wppl = stored.payload<SceneElement>();

    if (wppl.expired()) {
        return true;
    }

    /**
     *
     */
    auto nse = wppl.lock();
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
     *
     */
    _consumed.push_back({});

    /**
     *
     */
    return !node_.isLeaf();
}

void DepthModelPassProcessor::reset() {
    _consumed.clear();
}

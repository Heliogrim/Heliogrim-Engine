#include "DepthModelPassProcessor.hpp"

#include "../GraphicPass.hpp"

using namespace ember::engine::gfx;
using namespace ember;

DepthModelPassProcessor::DepthModelPassProcessor(ptr<const GraphicPass> graphicPass_) :
    ModelPassProcessor(graphicPass_) {}

bool DepthModelPassProcessor::operator()(cref<scene::SceneNode> node_) noexcept {

    /**
     * Precheck whether node has state or children
     */
    if (node_.isLeaf()) {
        return false;
    }

    /**
     * Check whether scene node as stored valid element reference; otherwise drop iteration
     */
    auto wpse = node_.element();

    if (wpse.expired()) {
        return true;
    }

    /**
     *
     */
    auto nse = wpse.lock();
    ref<scene::SceneElement> se = *nse;

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

#include "RevDepthPassModelProcessor.hpp"

#include "../../GraphicPass/GraphicPass.hpp"
#include "../../Scene/SceneNodeModel.hpp"

using namespace ember::engine::gfx;
using namespace ember;

RevDepthPassModelProcessor::RevDepthPassModelProcessor(ptr<const GraphicPass> graphicPass_) :
    GraphicPassModelProcessor(graphicPass_) {}

bool RevDepthPassModelProcessor::operator()(u32 batchIdx_, const ptr<scene::RenderGraph::node_type> node_) noexcept {

    /**
     *
     */
    if (node_->exclusiveSize()/* > 0*/) {

        assert(node_->isLeaf());

        auto** arr { node_->elements() };
        for (auto i = node_->exclusiveSize(); i > 0; --i, ++arr) {
            auto* entry { *arr };
            process(entry->batch(_graphicPass->mask()));
        }
    }

    /**
     * Precheck whether node has state or children
     */
    if (node_->isLeaf()) {
        return false;
    }

    // TODO: Remove!!!
    return !node_->isLeaf();

    #if FALSE
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
    #endif
}

void RevDepthPassModelProcessor::process(mref<ModelBatch> model_) noexcept {

    ProcessedModelBatch pmb {};

    pmb.geometry() = SharedGeometry {
        .vertices = model_.vertices,
        .instances = {},
        .indices = model_.indices
    };

    pmb.executions().push_back(ExecutionRow {
        DistinctBind {},
        DistinctGeometry {
            .transform = model_.transform,
            .instanceCount = 1,
            .instanceOffset = 0,
            .indexCount = model_.indexCount,
            .indexOffset = model_.indexOffset
        }
    });

    _consumed.push_back(_STD move(pmb));
}

void RevDepthPassModelProcessor::reset() {
    _consumed.clear();
}

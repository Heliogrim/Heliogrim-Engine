#pragma once

namespace hg::editor::graphics::graph {
    enum class NodeGraphPipelineMode {
        eForward,
        eDeferred
    };

    struct NodeGraphPipelineSpec {
        NodeGraphPipelineMode mode;
    };
}

#pragma once

namespace hg::editor::gfx::graph {
    enum class NodeGraphPipelineMode {
        eForward,
        eDeferred
    };

    struct NodeGraphPipelineSpec {
        NodeGraphPipelineMode mode;
    };
}

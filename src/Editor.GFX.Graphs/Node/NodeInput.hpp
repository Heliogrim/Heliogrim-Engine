#pragma once

#include <Engine.Common/String.hpp>

#include "__fwd.hpp"
#include "NodeInputSpec.hpp"

namespace hg::editor::gfx::graph {
    class NodeInput {
    public:
        using this_type = NodeInput;

    public:
        string _uniqueName;
        NodeInputSpec _spec;
    };
}

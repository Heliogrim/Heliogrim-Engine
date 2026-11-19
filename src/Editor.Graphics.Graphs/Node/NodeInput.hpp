#pragma once

#include <Engine.Common/__macro.hpp>

#include "NodeInputSpec.hpp"
#include "__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::editor::graphics::graph {
    class NodeInput {
    public:
        using this_type = NodeInput;

    public:
        string _uniqueName;
        NodeInputSpec _spec;
    };
}

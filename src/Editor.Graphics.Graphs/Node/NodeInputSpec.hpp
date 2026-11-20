#pragma once

#include "__fwd.hpp"
#include "NodeInputValueAttributes.hpp"
#include "NodeLinkValueType.hpp"

namespace hg::editor::graphics::graph {
    struct NodeInputSpec {
        NodeLinkValueType valueType;
        NodeInputValueAttributes valueAttributes;
    };
}

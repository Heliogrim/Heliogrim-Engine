#pragma once

#include "__fwd.hpp"

#include "NodeLinkValueType.hpp"
#include "NodeInputValueAttributes.hpp"

namespace hg::editor::graphics::graph {
    struct NodeInputSpec {
        NodeLinkValueType valueType;
        NodeInputValueAttributes valueAttributes;
    };
}

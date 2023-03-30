#pragma once

#include "__fwd.hpp"

#include "NodeLinkValueType.hpp"
#include "NodeOutputValueAttributes.hpp"

namespace hg::editor::gfx::graph {
    struct NodeOutputSpec {
        NodeLinkValueType valueType;
        NodeOutputValueAttributes valueAttributes;
    };
}

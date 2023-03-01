#pragma once

#include <Engine.Common/Math/Extent.hpp>

namespace hg::engine::reflow {
    struct FlowContext {
        math::fExtent2D viewport;
        math::fExtent2D scissor;
    };
}

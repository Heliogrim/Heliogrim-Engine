#pragma once

#include <Engine.Common/Math/Extent.hpp>

namespace ember::engine::reflow {

    struct FlowContext {
        math::fExtent2D viewport;
        math::fExtent2D scissor;
    };

}
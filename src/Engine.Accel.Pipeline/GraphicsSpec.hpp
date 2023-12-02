#pragma once

#include "AccelerationSpecification.hpp"

namespace hg::engine::accel {
    struct GraphicsSpec final {
        DepthCompareOp depthCompareOp;

        // TODO: Stencil States
    };
}

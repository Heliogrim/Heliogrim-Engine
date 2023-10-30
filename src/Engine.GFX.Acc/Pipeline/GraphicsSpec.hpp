#pragma once

#include "AccelerationSpecification.hpp"

namespace hg::engine::gfx::acc {
    struct GraphicsSpec final {
        DepthCompareOp depthCompareOp;

        // TODO: Stencil States
    };
}

#pragma once

#include "AccelerationSpecification.hpp"

namespace hg::engine::gfx::acc {
    struct AccelerationGraphicsSpec final {
        DepthCompareOp depthCompareOp;

        // TODO: Stencil States
    };
}

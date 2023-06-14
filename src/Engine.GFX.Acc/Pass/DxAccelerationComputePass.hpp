#pragma once

#include "AccelerationComputePass.hpp"

namespace hg::engine::gfx::acc {
    class DxAccelerationComputePass :
        public AccelerationComputePass {
    public:
        using this_type = DxAccelerationComputePass;

    public:
        ~DxAccelerationComputePass() override = default;
    };
}

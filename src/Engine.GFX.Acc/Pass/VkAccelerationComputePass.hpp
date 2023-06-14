#pragma once

#include "AccelerationComputePass.hpp"

namespace hg::engine::gfx::acc {
    class VkAccelerationComputePass :
        public AccelerationComputePass {
    public:
        using this_type = VkAccelerationComputePass;

    public:
        ~VkAccelerationComputePass() override = default;
    };
}

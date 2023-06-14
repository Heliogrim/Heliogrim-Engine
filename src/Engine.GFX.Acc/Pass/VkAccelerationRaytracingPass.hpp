#pragma once

#include "AccelerationRaytracingPass.hpp"

namespace hg::engine::gfx::acc {
    class VkAccelerationRaytracingPass :
        public AccelerationRaytracingPass {
    public:
        using this_type = VkAccelerationRaytracingPass;

    public:
        ~VkAccelerationRaytracingPass() override = default;
    };
}

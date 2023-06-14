#pragma once

#include "AccelerationMeshPass.hpp"

namespace hg::engine::gfx::acc {
    class VkAccelerationMeshPass :
        public AccelerationMeshPass {
    public:
        using this_type = VkAccelerationMeshPass;

    public:
        ~VkAccelerationMeshPass() override = default;
    };
}

#pragma once

#include "AccelerationGraphicsPass.hpp"

namespace hg::engine::gfx::acc {
    class VkAccelerationGraphicsPass :
        public AccelerationGraphicsPass {
    public:
        using this_type = VkAccelerationGraphicsPass;

    public:
        ~VkAccelerationGraphicsPass() override = default;
    };
}

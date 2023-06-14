#pragma once

#include "AccelerationRaytracingPass.hpp"

namespace hg::engine::gfx::acc {
    class DxAccelerationRaytracingPass :
        public AccelerationRaytracingPass {
    public:
        using this_type = DxAccelerationRaytracingPass;

    public:
        ~DxAccelerationRaytracingPass() override = default;
    };
}

#pragma once

#include "../AccelerationPass.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationComputePass :
        public AccelerationPass {
    public:
        using this_type = AccelerationComputePass;

    public:
        ~AccelerationComputePass() override = default;
    };
}

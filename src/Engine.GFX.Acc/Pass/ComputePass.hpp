#pragma once

#include "../AccelerationPass.hpp"

namespace hg::engine::gfx::acc {
    class ComputePass :
        public InheritMeta<ComputePass, AccelerationPass> {
    public:
        using this_type = ComputePass;

    public:
        ~ComputePass() override = default;
    };
}

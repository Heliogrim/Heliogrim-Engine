#pragma once

#include "../AccelerationPass.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationRaytracingPass :
        public InheritMeta<AccelerationRaytracingPass, AccelerationPass> {
    public:
        using this_type = AccelerationRaytracingPass;

    public:
        ~AccelerationRaytracingPass() override = default;
    };
}

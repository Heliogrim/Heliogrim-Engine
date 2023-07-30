#pragma once

#include "../AccelerationPass.hpp"

namespace hg::engine::gfx::acc {
    class RaytracingPass :
        public InheritMeta<RaytracingPass, AccelerationPass> {
    public:
        using this_type = RaytracingPass;

    public:
        ~RaytracingPass() override = default;
    };
}

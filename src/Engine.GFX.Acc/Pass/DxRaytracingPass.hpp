#pragma once

#include "RaytracingPass.hpp"

namespace hg::engine::gfx::acc {
    class DxRaytracingPass :
        public RaytracingPass {
    public:
        using this_type = DxRaytracingPass;

    public:
        ~DxRaytracingPass() override = default;
    };
}

#pragma once

#include "RaytracingPipeline.hpp"

namespace hg::engine::gfx::acc {
    class DxRaytracingPipeline :
        public RaytracingPipeline {
    public:
        using this_type = DxRaytracingPipeline;

    public:
        ~DxRaytracingPipeline() override = default;
    };
}

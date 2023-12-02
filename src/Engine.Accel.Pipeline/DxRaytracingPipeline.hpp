#pragma once

#include "RaytracingPipeline.hpp"

namespace hg::engine::accel {
    class DxRaytracingPipeline :
        public RaytracingPipeline {
    public:
        using this_type = DxRaytracingPipeline;

    public:
        ~DxRaytracingPipeline() override = default;
    };
}

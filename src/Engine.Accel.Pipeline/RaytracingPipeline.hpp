#pragma once

#include "AccelerationPipeline.hpp"

namespace hg::engine::accel {
    class RaytracingPipeline :
        public InheritMeta<RaytracingPipeline, AccelerationPipeline> {
    public:
        using this_type = RaytracingPipeline;

    public:
        ~RaytracingPipeline() override = default;
    };
}

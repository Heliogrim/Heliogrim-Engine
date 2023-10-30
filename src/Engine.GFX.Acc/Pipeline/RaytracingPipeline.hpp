#pragma once

#include "../AccelerationPipeline.hpp"

namespace hg::engine::gfx::acc {
    class RaytracingPipeline :
        public InheritMeta<RaytracingPipeline, AccelerationPipeline> {
    public:
        using this_type = RaytracingPipeline;

    public:
        ~RaytracingPipeline() override = default;
    };
}

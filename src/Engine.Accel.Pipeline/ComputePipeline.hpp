#pragma once

#include "AccelerationPipeline.hpp"

namespace hg::engine::accel {
    class ComputePipeline :
        public InheritMeta<ComputePipeline, AccelerationPipeline> {
    public:
        using this_type = ComputePipeline;

    public:
        ~ComputePipeline() override = default;
    };
}

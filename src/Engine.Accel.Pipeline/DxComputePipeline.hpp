#pragma once

#include "ComputePipeline.hpp"

namespace hg::engine::accel {
    class DxComputePipeline :
        public ComputePipeline {
    public:
        using this_type = DxComputePipeline;

    public:
        ~DxComputePipeline() override = default;
    };
}

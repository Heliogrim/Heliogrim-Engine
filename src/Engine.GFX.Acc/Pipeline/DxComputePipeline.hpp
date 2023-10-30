#pragma once

#include "ComputePipeline.hpp"

namespace hg::engine::gfx::acc {
    class DxComputePipeline :
        public ComputePipeline {
    public:
        using this_type = DxComputePipeline;

    public:
        ~DxComputePipeline() override = default;
    };
}

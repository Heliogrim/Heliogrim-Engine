#pragma once

#include "ComputePass.hpp"

namespace hg::engine::gfx::acc {
    class DxComputePass :
        public ComputePass {
    public:
        using this_type = DxComputePass;

    public:
        ~DxComputePass() override = default;
    };
}

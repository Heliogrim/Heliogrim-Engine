#pragma once

#include "AccelerationGraphicsPass.hpp"

namespace hg::engine::gfx::acc {
    class DxAccelerationGraphicsPass :
        public AccelerationGraphicsPass {
    public:
        using this_type = DxAccelerationGraphicsPass;

    public:
        ~DxAccelerationGraphicsPass() override = default;
    };
}

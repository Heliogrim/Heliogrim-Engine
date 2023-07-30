#pragma once

#include "GraphicsPass.hpp"

namespace hg::engine::gfx::acc {
    class DxAccelerationGraphicsPass :
        public GraphicsPass {
    public:
        using this_type = DxAccelerationGraphicsPass;

    public:
        ~DxAccelerationGraphicsPass() override = default;
    };
}

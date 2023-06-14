#pragma once

#include "AccelerationMeshPass.hpp"

namespace hg::engine::gfx::acc {
    class DxAccelerationMeshPass :
        public AccelerationMeshPass {
    public:
        using this_type = DxAccelerationMeshPass;

    public:
        ~DxAccelerationMeshPass() override = default;
    };
}

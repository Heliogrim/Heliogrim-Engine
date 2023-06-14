#pragma once

#include "../AccelerationPass.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationMeshPass :
        public AccelerationPass {
    public:
        using this_type = AccelerationMeshPass;

    public:
        ~AccelerationMeshPass() override = default;
    };
}

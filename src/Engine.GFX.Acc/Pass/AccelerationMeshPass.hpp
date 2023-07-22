#pragma once

#include "../AccelerationPass.hpp"

namespace hg::engine::gfx::acc {
    class AccelerationMeshPass :
        public InheritMeta<AccelerationMeshPass, AccelerationPass> {
    public:
        using this_type = AccelerationMeshPass;

    public:
        ~AccelerationMeshPass() override = default;
    };
}

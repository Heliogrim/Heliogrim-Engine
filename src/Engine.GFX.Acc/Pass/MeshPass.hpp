#pragma once

#include "../AccelerationPass.hpp"

namespace hg::engine::gfx::acc {
    class MeshPass :
        public InheritMeta<MeshPass, AccelerationPass> {
    public:
        using this_type = MeshPass;

    public:
        ~MeshPass() override = default;
    };
}

#pragma once

#include "MeshPass.hpp"

namespace hg::engine::gfx::acc {
    class DxMeshPass :
        public MeshPass {
    public:
        using this_type = DxMeshPass;

    public:
        ~DxMeshPass() override = default;
    };
}

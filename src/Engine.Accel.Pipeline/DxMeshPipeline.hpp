#pragma once

#include "MeshPipeline.hpp"

namespace hg::engine::accel {
    class DxMeshPipeline :
        public MeshPipeline {
    public:
        using this_type = DxMeshPipeline;

    public:
        ~DxMeshPipeline() override = default;
    };
}

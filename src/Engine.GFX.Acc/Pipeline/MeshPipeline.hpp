#pragma once

#include "../AccelerationPipeline.hpp"

namespace hg::engine::gfx::acc {
    class MeshPipeline :
        public InheritMeta<MeshPipeline, AccelerationPipeline> {
    public:
        using this_type = MeshPipeline;

    public:
        ~MeshPipeline() override = default;
    };
}

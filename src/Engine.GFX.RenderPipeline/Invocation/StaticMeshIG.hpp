#pragma once

#include <Engine.GFX.RenderGraph/Relation/MeshDescription.hpp>

#include "InvocationGenerator.hpp"

namespace hg::engine::gfx::render::pipeline {
    class StaticMeshIG :
        public InvocationGenerator {
    public:
        [[nodiscard]] static smr<graph::MeshDescription> getMeshDescription();
    };
}

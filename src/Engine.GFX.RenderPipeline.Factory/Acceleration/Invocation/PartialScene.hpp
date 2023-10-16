#pragma once
#include <Engine.GFX.RenderGraph/Node/SubpassNode.hpp>
#include <Engine.GFX.RenderPipeline/Invocation/SceneMeshIG.hpp>

namespace hg::engine::gfx::render::pipeline {
    class PartialScene {
    public:
        [[nodiscard]] smr<SceneMeshIG> operator()(
            mref<smr<graph::SubpassNode>> node_
        ) const noexcept;
    };
}

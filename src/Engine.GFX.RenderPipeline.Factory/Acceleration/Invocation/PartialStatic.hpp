#pragma once
#include <Engine.GFX.RenderGraph/Node/SubpassNode.hpp>
#include <Engine.GFX.RenderPipeline/Invocation/StaticMeshIG.hpp>
#include <Engine.GFX.RenderPipeline/Stage/MaterialSubpassStage.hpp>

namespace hg::engine::gfx::render::pipeline {
    class PartialStatic {
    public:
        [[nodiscard]] smr<StaticMeshIG> operator()(
            mref<smr<graph::SubpassNode>> node_
        ) const noexcept;
    };
}

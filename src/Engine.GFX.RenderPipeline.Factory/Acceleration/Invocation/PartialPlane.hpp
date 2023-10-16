#pragma once
#include <Engine.GFX.RenderGraph/Node/SubpassNode.hpp>
#include <Engine.GFX.RenderPipeline/Invocation/PlaneMeshIG.hpp>
#include <Engine.GFX.RenderPipeline/Stage/MaterialSubpassStage.hpp>

namespace hg::engine::gfx::render::pipeline {
    class PartialPlane {
    public:
        [[nodiscard]] smr<PlaneMeshIG> operator()(
            mref<smr<graph::SubpassNode>> node_
        ) const noexcept;
    };
}

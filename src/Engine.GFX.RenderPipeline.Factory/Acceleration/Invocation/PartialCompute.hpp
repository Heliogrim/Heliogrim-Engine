#pragma once
#include <Engine.GFX.RenderGraph/Node/SubpassNode.hpp>
#include <Engine.GFX.RenderPipeline/Invocation/ComputeIG.hpp>
#include <Engine.GFX.RenderPipeline/Stage/MaterialSubpassStage.hpp>

namespace hg::engine::gfx::render::pipeline {
    class PartialCompute {
    public:
        [[nodiscard]] smr<ComputeIG> operator()(
            mref<smr<graph::SubpassNode>> node_
        ) const noexcept;
    };
}

#pragma once
#include <Engine.GFX.RenderGraph/Node/SubpassNode.hpp>
#include <Engine.GFX.RenderPipeline/Invocation/ProcMeshIG.hpp>
#include <Engine.GFX.RenderPipeline/Stage/MaterialSubpassStage.hpp>

namespace hg::engine::gfx::render::pipeline {
    class PartialProcedural {
    public:
        [[nodiscard]] smr<ProcMeshIG> operator()(
            mref<smr<graph::SubpassNode>> node_
        ) const noexcept;
    };
}

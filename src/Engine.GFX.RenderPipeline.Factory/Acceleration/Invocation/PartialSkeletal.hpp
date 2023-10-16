#pragma once
#include <Engine.GFX.RenderGraph/Node/SubpassNode.hpp>
#include <Engine.GFX.RenderPipeline/Invocation/SkeletalMeshIG.hpp>
#include <Engine.GFX.RenderPipeline/Stage/MaterialSubpassStage.hpp>

namespace hg::engine::gfx::render::pipeline {
    class PartialSkeletal {
    public:
        [[nodiscard]] smr<SkeletalMeshIG> operator()(
            mref<smr<graph::SubpassNode>> node_
        ) const noexcept;
    };
}

#pragma once
#include <Engine.GFX.RenderGraph/Node/SubpassNode.hpp>
#include <Engine.GFX.RenderPipeline/Stage/MaterialSubpassStage.hpp>

namespace hg::engine::gfx::render::pipeline {
    class PartialMaterial {
    public:
        [[nodiscard]] smr<MaterialSubpassStage> operator()(
            mref<smr<graph::SubpassNode>> node_,
            mref<uptr<InvocationGenerator>> ig_
        ) const noexcept;
    };
}

#pragma once
#include <Engine.GFX.RenderGraph/Node/SubpassNode.hpp>
#include <Engine.GFX.RenderPipeline/Stage/AccelSubpassStage.hpp>

namespace hg::engine::gfx::render::pipeline {
    class PartialSingle {
    public:
        [[nodiscard]] smr<AccelSubpassStage> operator()(
            mref<smr<graph::SubpassNode>> node_,
            mref<uptr<InvocationGenerator>> ig_
        ) const noexcept;
    };
}

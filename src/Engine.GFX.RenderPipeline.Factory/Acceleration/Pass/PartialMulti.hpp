#pragma once
#include <Engine.GFX.RenderGraph/Node/SubpassNode.hpp>
#include <Engine.GFX.RenderPipeline/Stage/MultiAccelSubpassStage.hpp>

namespace hg::engine::gfx::render::pipeline {
    class PartialMulti {
    public:
        [[nodiscard]] smr<MultiAccelSubpassStage> operator()(
            mref<smr<graph::SubpassNode>> node_,
            mref<uptr<InvocationGenerator>> ig_
        ) const noexcept;
    };
}

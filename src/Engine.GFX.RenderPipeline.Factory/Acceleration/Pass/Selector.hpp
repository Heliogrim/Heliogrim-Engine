#pragma once
#include <Engine.GFX.RenderGraph/Node/SubpassNode.hpp>
#include <Engine.GFX.RenderPipeline/Stage/SubpassStage.hpp>

namespace hg::engine::gfx::render::pipeline {
    class AccelerationPassSelector {
    public:
        constexpr AccelerationPassSelector() noexcept = default;

        constexpr ~AccelerationPassSelector() noexcept = default;

    public:
        [[nodiscard]] smr<SubpassStage> operator()(
            mref<smr<graph::SubpassNode>> node_,
            mref<uptr<InvocationGenerator>> ig_
        ) const noexcept;
    };
}

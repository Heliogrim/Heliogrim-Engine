#pragma once
#include <Engine.GFX.RenderGraph/Node/SubpassNode.hpp>
#include <Engine.GFX.RenderPipeline/Invocation/InvocationGenerator.hpp>

namespace hg::engine::gfx::render::pipeline {
    class AccelerationInvocationSelector {
    public:
        constexpr AccelerationInvocationSelector() noexcept = default;

        constexpr ~AccelerationInvocationSelector() noexcept = default;

    public:
        [[nodiscard]] smr<InvocationGenerator> operator()(
            mref<smr<graph::SubpassNode>> node_
        ) const noexcept;
    };
}

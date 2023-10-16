#pragma once
#include <Engine.GFX.RenderGraph/RuntimeGraph.hpp>

#include "Engine.GFX.RenderPipeline/RenderPipeline.hpp"

namespace hg::engine::gfx::render::pipeline {
    class StreamBuilder {
    public:
        constexpr StreamBuilder() noexcept = default;

        constexpr ~StreamBuilder() noexcept = default;

    public:
        [[nodiscard]] uptr<RenderPipeline> operator()(
            mref<uptr<RenderPipeline>> iteratee_,
            mref<uptr<graph::RuntimeGraph>> runtimeGraph_
        ) const;
    };
}

#pragma once
#include <Engine.GFX.RenderGraph/RuntimeGraph.hpp>

#include "../RenderPipeline.hpp"

namespace hg::engine::gfx::render::pipeline {
    class BaseBuilder final {
    public:
        constexpr BaseBuilder() noexcept = default;

        constexpr ~BaseBuilder() noexcept = default;

    public:
        [[nodiscard]] uptr<RenderPipeline> operator()(mref<uptr<graph::RuntimeGraph>> runtimeGraph_) const;
    };
}

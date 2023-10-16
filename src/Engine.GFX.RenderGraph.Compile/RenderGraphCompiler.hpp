#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX.RenderGraph/CompileGraph.hpp>

#include "CompileRequest.hpp"

namespace hg::engine::gfx::render::graph {
    class RenderGraphCompiler {
    public:
        using this_type = RenderGraphCompiler;

    public:
        constexpr RenderGraphCompiler() noexcept = default;

        constexpr ~RenderGraphCompiler() noexcept = default;

    public:
        [[nodiscard]] uptr<RuntimeGraph> operator()(
            mref<CompileRequest> request_
        ) const;
    };
}

#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX.RenderGraph/CompileGraph.hpp>

#include "CompileRequest.hpp"

namespace hg::engine::gfx::render::graph {
    class RenderGraphCompiler {
    public:
        using this_type = RenderGraphCompiler;

    public:
        RenderGraphCompiler();

        ~RenderGraphCompiler();

    public:
        [[nodiscard]] uptr<RuntimeGraph> operator()(
            mref<CompileRequest> request_
        );
    };
}

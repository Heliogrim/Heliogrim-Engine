#pragma once

#include "CompileRequest.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Graphics.RenderGraph;
#endif

namespace hg::engine::render::graph {
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

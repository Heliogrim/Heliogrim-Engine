#pragma once

#include "ResolverPass.hpp"

namespace hg::engine::gfx::render::graph {
    class ResolverValidationPass final :
        public ResolverPass {
    public:
        using this_type = ResolverValidationPass;

    public:
        ResolverValidationPass() noexcept;

        ~ResolverValidationPass() override;

    public:
        [[nodiscard]] uptr<RenderGraph> operator()(mref<uptr<RenderGraph>> graph_) const override;
    };
}

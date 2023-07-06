#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "ResolverOptions.hpp"
#include "../__fwd.hpp"

namespace hg::engine::gfx::render::graph {
    class ResolverPass {
    public:
        using this_type = ResolverPass;

    protected:
        ResolverPass() noexcept = default;

    public:
        virtual ~ResolverPass() noexcept = default;

    protected:
        ResolverPassFlags _flags;

    public:
        [[nodiscard]] ResolverPassFlags getResolverFlags() const noexcept;

    public:
        [[nodiscard]] virtual uptr<RenderGraph> operator()(_In_ mref<uptr<RenderGraph>> graph_) const noexcept = 0;
    };
}

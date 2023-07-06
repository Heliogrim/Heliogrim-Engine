#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "../__fwd.hpp"
#include "ResolverPass.hpp"

namespace hg::engine::gfx::render::graph {
    class Resolver {
    public:
        using this_type = Resolver;

    public:
        Resolver() noexcept = default;

        ~Resolver() noexcept = default;

    private:
        Vector<uptr<ResolverPass>> _passes;

    public:
        [[nodiscard]] uptr<RenderGraph> operator()(
            _In_ mref<uptr<RenderGraph>> graph_,
            _In_ mref<ResolverOptions> options_
        ) const noexcept;
    };
}

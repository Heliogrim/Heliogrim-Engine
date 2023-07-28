#pragma once

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "../__fwd.hpp"

namespace hg::engine::gfx::render::graph {
    class Builder {
    public:
        using this_type = Builder;

    public:
        constexpr Builder() noexcept = default;

        constexpr ~Builder() noexcept = default;

    public:
        [[nodiscard]] static uptr<RenderGraph> insertNode(
            _In_ mref<uptr<RenderGraph>> graph_,
            nmpt<const Node> from_,
            nmpt<const Node> to_,
            _In_ mref<smr<Node>> node_
        );

        [[nodiscard]] static uptr<RenderGraph> insertSubGraph(
            _In_ mref<uptr<RenderGraph>> graph_,
            nmpt<const Node> from_,
            nmpt<const Node> to_,
            _In_ mref<smr<Node>> begin_,
            _In_ mref<smr<Node>> end_
        );

        [[nodiscard]] static uptr<RenderGraph> eraseNode(
            _In_ mref<uptr<RenderGraph>> graph_,
            nmpt<const Node> where_,
            _In_ mref<smr<Node>> node_
        );
    };
}

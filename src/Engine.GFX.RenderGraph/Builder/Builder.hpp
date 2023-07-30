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
        static void insertNode(
            nmpt<const Node> from_,
            nmpt<const Node> to_,
            _In_ mref<smr<Node>> node_,
            _Inout_ nmpt<RuntimeGraph> graph_
        );

        [[nodiscard]] static uptr<CompileGraph> insertNode(
            _In_ mref<uptr<CompileGraph>> graph_,
            nmpt<const Node> from_,
            nmpt<const Node> to_,
            _In_ mref<smr<Node>> node_
        );

        static void insertSubGraph(
            nmpt<const Node> from_,
            nmpt<const Node> to_,
            _In_ mref<smr<Node>> begin_,
            _In_ mref<smr<Node>> end_,
            _Inout_ nmpt<RuntimeGraph> graph_
        );

        [[nodiscard]] static uptr<CompileGraph> insertSubGraph(
            _In_ mref<uptr<CompileGraph>> graph_,
            nmpt<const Node> from_,
            nmpt<const Node> to_,
            _In_ mref<smr<Node>> begin_,
            _In_ mref<smr<Node>> end_
        );

        [[nodiscard]] static uptr<RuntimeGraph> eraseNode(
            _In_ mref<uptr<RuntimeGraph>> graph_,
            nmpt<const Node> where_,
            _In_ mref<smr<Node>> node_
        );

        [[nodiscard]] static uptr<CompileGraph> eraseNode(
            _In_ mref<uptr<CompileGraph>> graph_,
            nmpt<const Node> where_,
            _In_ mref<smr<Node>> node_
        );
    };
}

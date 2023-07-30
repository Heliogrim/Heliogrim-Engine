#pragma once
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include "Auxiliary.hpp"
#include "../__fwd.hpp"

namespace hg::engine::gfx::render::graph {
    class Node {
    public:
        using this_type = Node;

    protected:
        Node() noexcept = default;

        Node(mref<smr<Auxiliary>> auxiliary_) noexcept;

    public:
        virtual ~Node() noexcept = default;

    protected:
        smr<Auxiliary> _auxiliary;

    protected:
        void auxAdd(mref<uptr<Component>> comp_);

        void auxRem(mref<nmpt<Component>> comp_);

    public:
        [[nodiscard]] smr<Auxiliary> auxiliary() const noexcept;

    public:
        virtual void accept(ref<Visitor> visitor_) const = 0;

        virtual void traverse(ref<Visitor> visitor_) const = 0;
    };
}

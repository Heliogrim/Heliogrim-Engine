#pragma once

#include <Engine.Common/Collection/Vector.hpp>

#include "Node.hpp"

namespace hg::engine::gfx::render::graph {
    class ConvergeNode final :
        public Node {
    public:
        using this_type = ConvergeNode;

    public:
        ConvergeNode() noexcept = default;

        ~ConvergeNode() noexcept override = default;

    public:
        void accept(ref<Visitor> visitor_) const override;

        void traverse(ref<Visitor> visitor_) const override;

        void rtraverse(ref<Visitor> visitor_) const override;

    private:
        smr<Node> _next;
        Vector<nmpt<const Node>> _prev;

    public:
        void setNext(mref<smr<Node>> next_);

        [[nodiscard]] smr<Node> getNext() const noexcept;

        void addPrev(mref<nmpt<const Node>> prev_);

        void setPrev(mref<Vector<nmpt<const Node>>> prev_);

        void removePrev(cref<nmpt<const Node>> prev_);

        [[nodiscard]] cref<Vector<nmpt<const Node>>> getPrev() const noexcept;
    };
}

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
        Vector<smr<Node>> _prev;

    public:
        void setNext(mref<smr<Node>> next_);

        [[nodiscard]] smr<Node> getNext() const noexcept;

        void addPrev(mref<smr<Node>> prev_);

        void setPrev(mref<Vector<smr<Node>>> prev_);

        void removePrev(cref<smr<Node>> prev_);

        [[nodiscard]] cref<Vector<smr<Node>>> getPrev() const noexcept;
    };
}

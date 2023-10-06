#pragma once

#include <Engine.Common/Collection/Vector.hpp>

#include "Node.hpp"

namespace hg::engine::gfx::render::graph {
    class DivergeNode final :
        public Node {
    public:
        using this_type = DivergeNode;

    public:
        DivergeNode() noexcept = default;

        ~DivergeNode() noexcept override = default;

    public:
        void accept(ref<Visitor> visitor_) const override;

        void traverse(ref<Visitor> visitor_) const override;

        void rtraverse(ref<Visitor> visitor_) const override;

    private:
        Vector<smr<Node>> _next;
        smr<Node> _prev;

    public:
        void addNext(mref<smr<Node>> next_);

        void setNext(mref<Vector<smr<Node>>> next_);

        void removeNext(cref<smr<Node>> next_);

        [[nodiscard]] cref<Vector<smr<Node>>> getNext() const noexcept;

        void setPrev(mref<smr<Node>> prev_);

        [[nodiscard]] smr<Node> getPrev() const noexcept;
    };
}

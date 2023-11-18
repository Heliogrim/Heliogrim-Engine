#pragma once
#include "../RuntimeNode.hpp"

namespace hg::engine::gfx::render::graph {
    class __declspec(novtable) SLNode :
        public RuntimeNode {
    public:
        using this_type = SLNode;

    protected:
        SLNode() noexcept;

    public:
        ~SLNode() noexcept override = default;

    public:
        void accept(ref<Visitor> visitor_) const override;

        void traverse(ref<Visitor> visitor_) const override;

        void rtraverse(ref<Visitor> visitor_) const override;

    private:
        smr<Node> _next;
        nmpt<const Node> _prev;

    public:
        void setNext(mref<smr<Node>> next_);

        [[nodiscard]] smr<Node> getNext() const noexcept;

        void setPrev(mref<nmpt<const Node>> prev_);

        [[nodiscard]] nmpt<const Node> getPrev() const noexcept;
    };
}

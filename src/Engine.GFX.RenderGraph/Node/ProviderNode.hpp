#pragma once

#include "Node.hpp"

namespace hg::engine::gfx::render::graph {
    class ProviderNode :
        public Node {
    public:
        using this_type = ProviderNode;

    public:
        ProviderNode() noexcept;

        ~ProviderNode() noexcept override = default;

    public:
        void accept(ref<Visitor> visitor_) const override;

        void traverse(ref<Visitor> visitor_) const override;

        void rtraverse(ref<Visitor> visitor_) const override;

    private:
        smr<Node> _next;
        smr<Node> _prev;

    public:
        void setNext(mref<smr<Node>> next_);

        [[nodiscard]] smr<Node> getNext() const noexcept;

        void setPrev(mref<smr<Node>> prev_);

        [[nodiscard]] smr<Node> getPrev() const noexcept;

    public:
        [[nodiscard]] nmpt<const ProviderComponent> getProviderComponent() const noexcept;

        [[nodiscard]] nmpt<ProviderComponent> getProviderComponent();
    };
}

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

    private:
        smr<Node> _next;

    public:
        [[nodiscard]] bool empty() const noexcept;

        void setNext(mref<smr<Node>> next_);

        [[nodiscard]] smr<Node> getNext() const noexcept;

    public:
        [[nodiscard]] nmpt<ProviderComponent> getProviderComponent();
    };
}

#pragma once

#include "../RuntimeNode.hpp"

namespace hg::engine::gfx::render::graph {
    class ProviderNode final :
        public RuntimeNode {
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
        nmpt<const Node> _prev;

    public:
        void setNext(mref<smr<Node>> next_);

        [[nodiscard]] smr<Node> getNext() const noexcept;

        void setPrev(mref<nmpt<const Node>> prev_);

        [[nodiscard]] nmpt<const Node> getPrev() const noexcept;

    public:
        [[nodiscard]] nmpt<const ProviderComponent> getProviderComponent() const noexcept;

        [[nodiscard]] nmpt<ProviderComponent> getProviderComponent();

    public:
        [[nodiscard]] IterationResult iterate(cref<IterationPassContext> ctx_) noexcept override;

        [[nodiscard]] IterationResult resolve(cref<ResolvePassContext> ctx_) noexcept override;

        [[nodiscard]] IterationResult execute(cref<ExecutionPassContext> ctx_) noexcept override;
    };
}

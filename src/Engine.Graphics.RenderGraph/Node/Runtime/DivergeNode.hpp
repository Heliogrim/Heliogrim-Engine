#pragma once

#include <Engine.Common/Collection/Vector.hpp>

#include "../RuntimeNode.hpp"

namespace hg::engine::render::graph {
    class DivergeNode final :
        public RuntimeNode {
    public:
        using this_type = DivergeNode;

    public:
        DivergeNode() noexcept;

        ~DivergeNode() noexcept override = default;

    public:
        void accept(ref<Visitor> visitor_) const override;

        void traverse(ref<Visitor> visitor_) const override;

        void rtraverse(ref<Visitor> visitor_) const override;

    private:
        Vector<smr<Node>> _next;
        nmpt<const Node> _prev;

    public:
        void addNext(mref<smr<Node>> next_);

        void setNext(mref<Vector<smr<Node>>> next_);

        void removeNext(cref<smr<Node>> next_);

        [[nodiscard]] cref<Vector<smr<Node>>> getNext() const noexcept;

        void setPrev(mref<nmpt<const Node>> prev_);

        [[nodiscard]] nmpt<const Node> getPrev() const noexcept;

    public:
        [[nodiscard]] IterationResult iterate(cref<IterationPassContext> ctx_) noexcept override;

        [[nodiscard]] IterationResult resolve(cref<ResolvePassContext> ctx_) noexcept override;

        [[nodiscard]] IterationResult execute(cref<ExecutionPassContext> ctx_) noexcept override;
    };
}

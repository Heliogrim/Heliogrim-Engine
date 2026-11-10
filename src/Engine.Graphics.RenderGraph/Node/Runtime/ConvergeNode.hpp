#pragma once

#include <Engine.Common/Collection/Vector.hpp>

#include "../RuntimeNode.hpp"

namespace hg::engine::render::graph {
    class ConvergeNode :
        public RuntimeNode {
    public:
        using this_type = ConvergeNode;

    public:
        ConvergeNode() noexcept;

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

    public:
        [[nodiscard]] IterationResult iterate(cref<IterationPassContext> ctx_) noexcept override;

        [[nodiscard]] IterationResult resolve(cref<ResolvePassContext> ctx_) noexcept override;

        [[nodiscard]] IterationResult execute(cref<ExecutionPassContext> ctx_) noexcept override;
    };
}

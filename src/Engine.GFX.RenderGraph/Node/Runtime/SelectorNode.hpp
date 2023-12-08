#pragma once

#include <bitset>
#include <Engine.Common/Collection/Vector.hpp>

#include "../RuntimeNode.hpp"

namespace hg::engine::render::graph {
    class SelectorNode final :
        public RuntimeNode {
    public:
        using this_type = SelectorNode;

        using mask_type = _STD bitset<16>;

    public:
        SelectorNode() noexcept;

        ~SelectorNode() noexcept override = default;

    public:
        void accept(ref<Visitor> visitor_) const override;

        void traverse(ref<Visitor> visitor_) const override;

        void rtraverse(ref<Visitor> visitor_) const override;

    public:
        struct Next final {
            mask_type mask;
            smr<Node> node;
        };

    private:
        Vector<Next> _next;
        nmpt<const Node> _prev;

    public:
        void addNext(mask_type mask, mref<smr<Node>> next_);

        void addNext(bool active_, mref<smr<Node>> next_);

        void removeNext(cref<smr<Node>> next_);

        [[nodiscard]] cref<Vector<Next>> getNext() const noexcept;

        void setPrev(mref<nmpt<const Node>> prev_);

        [[nodiscard]] nmpt<const Node> getPrev() const noexcept;

    public:
        [[nodiscard]] IterationResult iterate(cref<IterationPassContext> ctx_) noexcept override;

        [[nodiscard]] IterationResult resolve(cref<ResolvePassContext> ctx_) noexcept override;

        [[nodiscard]] IterationResult execute(cref<ExecutionPassContext> ctx_) noexcept override;
    };
}

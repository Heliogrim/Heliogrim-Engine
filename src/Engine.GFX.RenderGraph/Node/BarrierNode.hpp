#pragma once
#include "Node.hpp"
#include "../__fwd.hpp"

namespace hg::engine::gfx::render::graph {
    class BarrierNode final :
        public Node {
    public:
        using this_type = BarrierNode;

    public:
        BarrierNode() noexcept;

        ~BarrierNode() noexcept override = default;

    public:
        void accept(ref<Visitor> visitor_) const override;

        void traverse(ref<Visitor> visitor_) const override;

    private:
        smr<Node> _next;

    public:
        void setNext(mref<smr<Node>> next_);

        [[nodiscard]] smr<Node> getNext() const noexcept;

    public:
        [[nodiscard]] nmpt<const BarrierComponent> getBarrierComponent() const noexcept;

        [[nodiscard]] nmpt<BarrierComponent> getBarrierComponent();
    };
}

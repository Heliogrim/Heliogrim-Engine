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
        void traverse(ref<Visitor> visitor_) override;

    private:
        smr<Node> _next;

    public:
        void setNext(mref<smr<Node>> child_);

    public:
        [[nodiscard]] nmpt<BarrierComponent> getBarrierComponent();
    };
}

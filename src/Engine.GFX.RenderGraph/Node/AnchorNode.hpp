#pragma once
#include "Node.hpp"

namespace hg::engine::gfx::render::graph {
    class AnchorNode final :
        public Node {
    public:
        using this_type = AnchorNode;

    public:
        AnchorNode() noexcept;

        ~AnchorNode() noexcept override = default;

    public:
        void traverse(ref<Visitor> visitor_) override;

    private:
        smr<Node> _next;

    public:
        [[nodiscard]] bool empty() const noexcept;

        void setNext(mref<smr<Node>> next_);
    };
}

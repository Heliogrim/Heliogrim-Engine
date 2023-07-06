#pragma once

#include <Engine.Common/Collection/Vector.hpp>

#include "Node.hpp"

namespace hg::engine::gfx::render::graph {
    class ConvergeNode final :
        public Node {
    public:
        using this_type = ConvergeNode;

    public:
        ConvergeNode() noexcept = default;

        ~ConvergeNode() noexcept override = default;

    public:
        void traverse(ref<Visitor> visitor_) override;

    private:
        smr<Node> _next;

    public:
        [[nodiscard]] bool empty() const noexcept;

        void setNext(mref<smr<Node>> next_);
    };
}

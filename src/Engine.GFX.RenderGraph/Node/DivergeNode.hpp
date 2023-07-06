#pragma once

#include <Engine.Common/Collection/Vector.hpp>

#include "Node.hpp"

namespace hg::engine::gfx::render::graph {
    class DivergeNode final :
        public Node {
    public:
        using this_type = DivergeNode;

    public:
        DivergeNode() noexcept = default;

        ~DivergeNode() noexcept override = default;

    public:
        void traverse(ref<Visitor> visitor_) override;

    private:
        Vector<smr<Node>> _next;

    public:
        void addNext(mref<smr<Node>> next_);
    };
}

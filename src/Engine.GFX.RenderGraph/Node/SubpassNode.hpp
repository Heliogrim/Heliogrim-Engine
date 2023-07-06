#pragma once

#include "Node.hpp"
#include "../__fwd.hpp"

namespace hg::engine::gfx::render::graph {
    enum class SubpassAccelMode : bool {
        eSingle = false,
        eMulti = true
    };

    class SubpassNode final :
        public Node {
    public:
        using this_type = SubpassNode;

    public:
        SubpassNode(SubpassAccelMode mode_) noexcept;

        ~SubpassNode() noexcept override;

    public:
        void traverse(ref<Visitor> visitor_) override;

    private:
        smr<Node> _next;

    public:
        void setNext(mref<smr<Node>> next_);

    public:
        [[nodiscard]] nmpt<SubpassComponent> getSubpassComponent() const noexcept;

        [[nodiscard]] nmpt<SubpassAccelComponent> getSubpassAcceleration() const noexcept;
    };
}

#pragma once

#include "Node.hpp"
#include "../__fwd.hpp"

namespace hg::engine::gfx::render::graph {
    enum class SubpassAccelMode {
        eSingle = 0x0,
        eMulti = 0x1,
        eMaterial = 0x2
    };

    class SubpassNode final :
        public Node {
    public:
        using this_type = SubpassNode;

    public:
        SubpassNode(SubpassAccelMode mode_) noexcept;

        ~SubpassNode() noexcept override;

    public:
        void accept(ref<Visitor> visitor_) const override;

        void traverse(ref<Visitor> visitor_) const override;

    private:
        smr<Node> _next;

    public:
        void setNext(mref<smr<Node>> next_);

        [[nodiscard]] smr<Node> getNext() const noexcept;

    public:
        [[nodiscard]] nmpt<SubpassComponent> getSubpassComponent() const noexcept;

        [[nodiscard]] nmpt<SubpassAccelComponent> getSubpassAcceleration() const noexcept;
    };
}

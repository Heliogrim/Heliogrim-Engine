#pragma once

#include <bitset>
#include <Engine.Common/Collection/Vector.hpp>

#include "Node.hpp"

namespace hg::engine::gfx::render::graph {
    class SelectorNode final :
        public Node {
    public:
        using this_type = SelectorNode;

        using mask_type = _STD bitset<16>;

    public:
        SelectorNode() noexcept = default;

        ~SelectorNode() noexcept override = default;

    public:
        void accept(ref<Visitor> visitor_) const override;

        void traverse(ref<Visitor> visitor_) const override;

    public:
        struct Next final {
            mask_type mask;
            smr<Node> node;
        };

    private:
        Vector<Next> _next;

    public:
        void addNext(mask_type mask, mref<smr<Node>> next_);

        void addNext(bool active_, mref<smr<Node>> next_);

        [[nodiscard]] cref<Vector<Next>> getNext() const noexcept;
    };
}

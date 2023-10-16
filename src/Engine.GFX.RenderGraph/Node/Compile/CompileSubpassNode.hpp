#pragma once

#include "CompileNode.hpp"
#include "../SubpassNode.hpp"

namespace hg::engine::gfx::render::graph {
    class CompileSubpassNode final :
        public CompileNode {
    public:
        using this_type = CompileSubpassNode;

    public:
        CompileSubpassNode(SubpassAccelMode mode_) noexcept;

        ~CompileSubpassNode() override = default;

    public:
        void accept(ref<Visitor> visitor_) const override;

        void traverse(ref<Visitor> visitor_) const override;

        void rtraverse(ref<Visitor> visitor_) const override;

    private:
        smr<Node> _next;
        nmpt<const Node> _prev;

    public:
        void setNext(mref<smr<Node>> next_);

        [[nodiscard]] smr<Node> getNext() const noexcept;

        void setPrev(mref<nmpt<const Node>> prev_);

        [[nodiscard]] nmpt<const Node> getPrev() const noexcept;

    public:
        [[nodiscard]] nmpt<CompileSubpassComponent> getSubpassComponent() const noexcept;

        [[nodiscard]] nmpt<CompileSubpassAccelComponent> getSubpassAcceleration() const noexcept;
    };
}

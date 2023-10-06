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
        smr<Node> _prev;

    public:
        void setNext(mref<smr<Node>> next_);

        [[nodiscard]] smr<Node> getNext() const noexcept;

        void setPrev(mref<smr<Node>> prev_);

        [[nodiscard]] smr<Node> getPrev() const noexcept;

    public:
        [[nodiscard]] nmpt<CompileSubpassComponent> getSubpassComponent() const noexcept;

        [[nodiscard]] nmpt<CompileSubpassAccelComponent> getSubpassAcceleration() const noexcept;
    };
}

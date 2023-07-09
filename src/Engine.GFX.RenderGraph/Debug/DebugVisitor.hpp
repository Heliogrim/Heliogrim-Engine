#pragma once

#include "../Visitor/Visitor.hpp"

namespace hg::engine::gfx::render::graph {
    class DebugVisitor :
        public Visitor {
    public:
        using this_type = DebugVisitor;

    public:
        DebugVisitor() noexcept = default;

        ~DebugVisitor() noexcept override;

    public:
        void operator()(cref<Node> node_) override;

    public:
        void operator()(cref<AnchorNode> node_) override;

        void operator()(cref<BarrierNode> node_) override;

        void operator()(cref<ConvergeNode> node_) override;

        void operator()(cref<DivergeNode> node_) override;

        void operator()(cref<SelectorNode> node_) override;

        void operator()(cref<ProviderNode> node_) override;

        void operator()(cref<SubpassNode> node_) override;
    };
}

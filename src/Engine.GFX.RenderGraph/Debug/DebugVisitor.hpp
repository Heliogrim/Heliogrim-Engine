#pragma once

#include <Engine.Common/Collection/DenseSet.hpp>

#include "../Visitor/Visitor.hpp"

namespace hg::engine::render::graph {
    class DebugVisitor :
        public Visitor {
    public:
        using this_type = DebugVisitor;

    public:
        DebugVisitor() noexcept;

        ~DebugVisitor() noexcept override;

    private:
        DenseSet<const void*> _seen;

    public:
        void operator()(cref<Node> node_) override;

    public:
        void operator()(cref<AnchorNode> node_) override;

        void operator()(cref<BarrierNode> node_) override;

        void operator()(cref<ConvergeNode> node_) override;

        void operator()(cref<DivergeNode> node_) override;

        void operator()(cref<SelectorNode> node_) override;

        void operator()(cref<ProviderNode> node_) override;

        void operator()(cref<SubPassNodeBase> node_) override;

    public:
        void operator()(cref<CompileNode> node_) override;

    public:
        void operator()(cref<CompileSubPassNode> node_) override;
    };
}

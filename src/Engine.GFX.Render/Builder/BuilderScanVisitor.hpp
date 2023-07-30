#pragma once
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.GFX.RenderGraph/Visitor/Visitor.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

namespace hg::engine::gfx::render {
    class BuilderScanVisitor :
        public graph::Visitor {
    public:
        using this_type = BuilderScanVisitor;

    public:
        ~BuilderScanVisitor() noexcept override;

    private:
        DenseSet<ptr<const graph::Node>> _seen;
        DenseMap<ptr<const graph::Node>, DenseSet<ptr<const graph::Node>>> _nodeRc;

    public:
        [[nodiscard]] decltype(_nodeRc) extractNodeRcMap() noexcept;

        [[nodiscard]] decltype(_seen) extractSeenSet() noexcept;

    private:
        void mark(cref<graph::Node> origin_, cref<graph::Node> node_);

        [[nodiscard]] bool seen(cref<graph::Node> node_) const noexcept;

    public:
        void operator()(cref<graph::Node> node_) override;

    public:
        void operator()(cref<graph::AnchorNode> node_) override;

        void operator()(cref<graph::BarrierNode> node_) override;

        void operator()(cref<graph::ConvergeNode> node_) override;

        void operator()(cref<graph::DivergeNode> node_) override;

        void operator()(cref<graph::SelectorNode> node_) override;

        void operator()(cref<graph::ProviderNode> node_) override;

        void operator()(cref<graph::SubpassNode> node_) override;

    public:
        void operator()(cref<graph::CompileNode> node_) override;
    };
}

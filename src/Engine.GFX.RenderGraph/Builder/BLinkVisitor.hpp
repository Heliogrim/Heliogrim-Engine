#pragma once
#include "../Visitor/Visitor.hpp"
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "LinkMode.hpp"

namespace hg::engine::gfx::render::graph {
    class BLinkVisitor final :
        public Visitor {
    public:
        BLinkVisitor() noexcept;

        BLinkVisitor(mref<nmpt<const Node>> link_, LinkMode mode_ = LinkMode::eInclusive) noexcept;

        ~BLinkVisitor() override = default;

    private:
        LinkMode _linkMode;
        nmpt<const Node> _linkPredecessor;

    public:
        nmpt<const Node> getLinkPredecessor() const noexcept;

        void setLinkPredecessor(mref<nmpt<const Node>> link_);

    public:
        void operator()(cref<Node> node_) override;

    public:
        void operator()(cref<AnchorNode> node_) override;

        void operator()(cref<BarrierNode> node_) override;

        void operator()(cref<ConvergeNode> node_) override;

        void operator()(cref<DivergeNode> node_) override;

        void operator()(cref<SelectorNode> node_) override;

        void operator()(cref<ProviderNode> node_) override;

        void operator()(cref<SLNode> node_) override;

        void operator()(cref<SubPassNodeBase> node_) override;

    public:
        void operator()(cref<CompileNode> node_) override;

    public:
        void operator()(cref<CompileSubPassNode> node_) override;
    };
}

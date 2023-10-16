#pragma once
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "LinkMode.hpp"
#include "../Visitor/Visitor.hpp"

namespace hg::engine::gfx::render::graph {
    class FLinkVisitor final :
        public Visitor {
    public:
        FLinkVisitor() noexcept;

        FLinkVisitor(mref<smr<Node>> link_, LinkMode mode_ = LinkMode::eInclusive) noexcept;

        ~FLinkVisitor() override = default;

    private:
        LinkMode _linkMode;
        smr<Node> _linkSuccessor;

    public:
        [[nodiscard]] smr<Node> getLinkSuccessor() const;

        void setLinkSuccessor(mref<smr<Node>> link_);

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

    public:
        void operator()(cref<CompileNode> node_) override;

    public:
        void operator()(cref<CompileSubpassNode> node_) override;
    };
}

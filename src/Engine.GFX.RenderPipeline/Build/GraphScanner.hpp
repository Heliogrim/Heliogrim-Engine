#pragma once
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.GFX.RenderGraph/Visitor/Visitor.hpp>

namespace hg::engine::gfx::render::pipeline {
    class GraphScanner final :
        public graph::Visitor {
    public:
        GraphScanner() noexcept;

        ~GraphScanner() noexcept override;

    private:
        DenseSet<ptr<const graph::Node>> _seen;
        DenseMap<ptr<const graph::Node>, DenseSet<ptr<const graph::Node>>> _scannedDependencies;

    public:
        [[nodiscard]] decltype(_seen) extractSeenNodes() noexcept;

        [[nodiscard]] decltype(_scannedDependencies) extractNodeDependencies() noexcept;

    private:
        void mark(cref<graph::Node> subject_, cref<graph::Node> dependency_);

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

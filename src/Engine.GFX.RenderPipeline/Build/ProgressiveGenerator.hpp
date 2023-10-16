#pragma once
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.RenderGraph/Visitor/Visitor.hpp>

namespace hg::engine::gfx::render::pipeline {
    class Stage;

    class ProgressiveGenerator final :
        public graph::Visitor {
    public:
        ProgressiveGenerator() noexcept;

        ~ProgressiveGenerator() noexcept override;

    private:
        DenseMap<ptr<const graph::Node>, DenseSet<ptr<const graph::Node>>> _knownDependencies;
        DenseMap<ptr<const graph::Node>, smr<Stage>> _nodeToStage;

    public:
        void storeKnownDependencies(mref<decltype(_knownDependencies)> dependencies_);

        [[nodiscard]] decltype(_knownDependencies) extractKnownDependencies() noexcept;

        [[nodiscard]] decltype(_nodeToStage) extractProgressiveMapping() noexcept;

        [[nodiscard]] smr<Stage> findStage(const ptr<const graph::Node> node_) noexcept;

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

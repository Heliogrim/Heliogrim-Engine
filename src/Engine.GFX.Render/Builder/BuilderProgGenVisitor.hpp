#pragma once
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.RenderGraph/Visitor/Visitor.hpp>
#include <Engine.GFX.RenderPipeline/__fwd.hpp>

namespace hg::engine::gfx::render {
    /**
     * The progressive generator visitor is part of the building process, used to transform
     *  graph nodes type dependent into related stages.
     */
    class BuilderProgGenVisitor :
        public graph::Visitor {
    public:
        using this_type = BuilderProgGenVisitor;

    public:
        BuilderProgGenVisitor() noexcept;

        ~BuilderProgGenVisitor() override;

    private:
        DenseMap<ptr<const graph::Node>, DenseSet<ptr<const graph::Node>>> _nodeRcDep;
        DenseMap<ptr<const graph::Node>, smr<pipeline::Stage>> _mapping;

    private:
        [[nodiscard]] smr<pipeline::Stage> recursiveDepLookup(ptr<const graph::Node> initial_);

    public:
        void setNodeRcDep(mref<decltype(_nodeRcDep)> map_);

        [[nodiscard]] cref<decltype(_mapping)> getNodeStageMapping();

        [[nodiscard]] decltype(_mapping) extractNodeStageMapping();

        [[nodiscard]] smr<pipeline::Stage> findStage(ptr<const graph::Node> node_);

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

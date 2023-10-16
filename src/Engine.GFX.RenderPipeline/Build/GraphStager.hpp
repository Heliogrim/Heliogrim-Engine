#pragma once
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.Common/Collection/Queue.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.RenderGraph/Visitor/Visitor.hpp>

namespace hg::engine::gfx::render::pipeline {
    class GraphStager final :
        public graph::Visitor {
    public:
        GraphStager() noexcept;

        ~GraphStager() noexcept override;

    private:
        DenseMap<ptr<const graph::Node>, DenseSet<ptr<const graph::Node>>> _knownDependencies;
        Vector<Vector<nmpt<const graph::Node>>> _stages;

        DenseSet<ptr<const graph::Node>> _staged;
        Queue<ptr<const graph::Node>> _backlog;
        Vector<ptr<const graph::Node>> _deferred;

    public:
        [[nodiscard]] decltype(_knownDependencies) extractKnownDependencies() noexcept;

        void storeKnownDependencies(mref<decltype(_knownDependencies)> dependencies_);

        [[nodiscard]] decltype(_stages) extractStages() noexcept;

    private:
        void stageNode(cref<graph::Node> node_);

    public:
        void operator()(cref<graph::Node> node_) override;

        void operator()(cref<graph::CompileNode> node_) override;
    };
}

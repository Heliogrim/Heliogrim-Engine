#pragma once
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/DenseSet.hpp>
#include <Engine.Common/Collection/Queue.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX.RenderGraph/Visitor/Visitor.hpp>
#include <Engine.GFX.RenderGraph/__fwd.hpp>

namespace hg::engine::gfx::render {
    class BuilderLayerVisitor :
        public graph::Visitor {
    public:
        using this_type = BuilderLayerVisitor;

    public:
        ~BuilderLayerVisitor() noexcept override;

    private:
        DenseMap<ptr<const graph::Node>, DenseSet<ptr<const graph::Node>>> _nodeRcDep;

    public:
        void setNodeRcDep(mref<decltype(_nodeRcDep)> map_);

        [[nodiscard]] decltype(_nodeRcDep) extractNodeRcDep();

    private:
        u32 _layer = 0;
        Vector<Vector<nmpt<const graph::Node>>> _layers;

        DenseSet<ptr<const graph::Node>> _stored;
        Queue<ptr<const graph::Node>> _backlog;

    public:
        [[nodiscard]] cref<Vector<Vector<nmpt<const graph::Node>>>> layers() const;

    private:
        [[nodiscard]] bool updateSatisfied(ptr<const graph::Node> node_);

        void streamUnwind();

        void assure(u32 layer_);

        void pushAssured(ptr<const graph::Node> node_);

    public:
        void operator()(cref<graph::Node> node_) override;
    };
}

#include "Builder.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

#include "BuilderVisitor.hpp"
#include "../RenderGraph.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

uptr<engine::gfx::render::RenderGraph> Builder::insertNode(
    mref<uptr<RenderGraph>> graph_,
    nmpt<const Node> where_,
    mref<smr<Node>> node_
) {
    return insertSubGraph(_STD move(graph_), _STD move(where_), clone(node_), _STD move(node_));
}

uptr<engine::gfx::render::RenderGraph> Builder::insertSubGraph(
    mref<uptr<RenderGraph>> graph_,
    nmpt<const Node> where_,
    mref<smr<Node>> begin_,
    mref<smr<Node>> end_
) {

    BuilderVisitor visitor {
        [where_](cref<Node> node_) {
            return _STD addressof(node_) == where_.get();
        },
        _STD move(begin_),
        _STD move(end_)
    };

    graph_->update(visitor);
    return graph_;
}

uptr<engine::gfx::render::RenderGraph> Builder::eraseNode(
    mref<uptr<RenderGraph>> graph_,
    nmpt<const Node> where_,
    mref<smr<Node>> node_
) {
    throw NotImplementedException();
    return graph_;
}

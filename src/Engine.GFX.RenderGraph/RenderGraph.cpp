#include "RenderGraph.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx;
using namespace hg;

nmpt<const AnchorNode> render::RenderGraph::begin() const noexcept {
    return _begin;
}

nmpt<const AnchorNode> render::RenderGraph::end() const noexcept {
    return _end;
}

ref<render::RenderGraph::this_type> render::RenderGraph::pushNode(
    nmpt<const Node> where_,
    mref<smr<Node>> node_
) {
    return *this;
}

ref<render::RenderGraph::this_type> render::RenderGraph::pullNode(
    mref<nmpt<const Node>> node_
) {
    return *this;
}

void render::RenderGraph::update(cref<Visitor> visitor_) {}

Vector<Provision> render::RenderGraph::aggregateProvisions() const noexcept {
    return {};
}

Vector<Requirement> render::RenderGraph::aggregateRequirements() const noexcept {
    return {};
}

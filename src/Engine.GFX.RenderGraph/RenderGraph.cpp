#include "RenderGraph.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::gfx::render::graph;
using namespace hg::engine::gfx;
using namespace hg;

[[nodiscard]] static _STD pair<smr<AnchorNode>, smr<AnchorNode>> generateMinimalGraph() {

    auto pair = _STD make_pair(
        make_smr<AnchorNode>(),
        make_smr<AnchorNode>()
    );

    pair.first->setNext(clone(pair.second));
    return pair;
}

render::RenderGraph::RenderGraph() {
    auto pair = generateMinimalGraph();
    _begin = _STD move(pair.first);
    _end = _STD move(pair.second);
}

render::RenderGraph::RenderGraph(mref<smr<AnchorNode>> begin_, mref<smr<AnchorNode>> end_) noexcept :
    _begin(_STD move(begin_)),
    _end(_STD move(end_)) {}

nmpt<const AnchorNode> render::RenderGraph::begin() const noexcept {
    return _begin.get();
}

smr<AnchorNode> render::RenderGraph::begin() {
    return _begin;
}

nmpt<const AnchorNode> render::RenderGraph::end() const noexcept {
    return _end.get();
}

smr<AnchorNode> render::RenderGraph::end() {
    return _end;
}

void render::RenderGraph::update(ref<Visitor> visitor_) {
    assert(_begin);
    _begin->accept(visitor_);
}

Vector<Provision> render::RenderGraph::aggregateProvisions() const noexcept {
    return {};
}

Vector<Requirement> render::RenderGraph::aggregateRequirements() const noexcept {
    return {};
}

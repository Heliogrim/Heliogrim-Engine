#include "RuntimeGraph.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

using namespace hg::engine::render::graph;
using namespace hg;

[[nodiscard]] static std::pair<smr<AnchorNode>, smr<AnchorNode>> generateMinimalGraph() {

    auto pair = std::make_pair(
        make_smr<AnchorNode>(),
        make_smr<AnchorNode>()
    );

    pair.first->setNext(clone(pair.second));
    return pair;
}

RuntimeGraph::RuntimeGraph() {
    auto pair = generateMinimalGraph();
    _begin = std::move(pair.first);
    _end = std::move(pair.second);
}

RuntimeGraph::RuntimeGraph(mref<smr<AnchorNode>> begin_, mref<smr<AnchorNode>> end_) noexcept :
    _begin(std::move(begin_)),
    _end(std::move(end_)) {}

nmpt<const AnchorNode> RuntimeGraph::begin() const noexcept {
    return _begin.get();
}

smr<AnchorNode> RuntimeGraph::begin() {
    return _begin;
}

nmpt<const AnchorNode> RuntimeGraph::end() const noexcept {
    return _end.get();
}

smr<AnchorNode> RuntimeGraph::end() {
    return _end;
}

void RuntimeGraph::update(ref<Visitor> visitor_) {
    assert(_begin);
    _begin->accept(visitor_);
}

Vector<Provision> RuntimeGraph::aggregateProvisions() const noexcept {
    return {};
}

Vector<Requirement> RuntimeGraph::aggregateRequirements() const noexcept {
    return {};
}

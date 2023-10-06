#include "CompileGraph.hpp"

#include <Engine.Common/Collection/DenseSet.hpp>
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

CompileGraph::CompileGraph() {
    auto pair = generateMinimalGraph();
    _begin = _STD move(pair.first);
    _end = _STD move(pair.second);
}

CompileGraph::CompileGraph(mref<smr<AnchorNode>> begin_, mref<smr<AnchorNode>> end_) noexcept :
    _begin(_STD move(begin_)),
    _end(_STD move(end_)) {}

nmpt<const AnchorNode> CompileGraph::begin() const noexcept {
    return _begin.get();
}

smr<AnchorNode> CompileGraph::begin() {
    return _begin;
}

nmpt<const AnchorNode> CompileGraph::end() const noexcept {
    return _end.get();
}

smr<AnchorNode> CompileGraph::end() {
    return _end;
}

void CompileGraph::update(ref<Visitor> visitor_) {
    assert(_begin);
    _begin->accept(visitor_);
}

void CompileGraph::reverse(ref<Visitor> visitor_) {
    assert(_end);
    _end->accept(visitor_);
}

bool CompileGraph::addExpectedProvision(mref<Provision> expected_) {
    const auto result = _expectedProvision.insert(_STD move(expected_));
    return result.second;
}

bool CompileGraph::addExpectedRequirement(mref<Requirement> expected_) {
    const auto result = _expectedRequirement.insert(_STD move(expected_));
    return result.second;
}

_STD span<const Provision> CompileGraph::expectedProvision() const noexcept {
    return _STD span(
        _expectedProvision.begin(),
        _STD distance(_expectedProvision.begin(), _expectedProvision.end())
    );
}

_STD span<const Requirement> CompileGraph::expectedRequirement() const noexcept {
    return _STD span(
        _expectedRequirement.begin(),
        _STD distance(_expectedRequirement.begin(), _expectedRequirement.end())
    );
}

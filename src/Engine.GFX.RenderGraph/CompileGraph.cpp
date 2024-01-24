#include "CompileGraph.hpp"

#include <Engine.Common/Collection/DenseSet.hpp>
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

CompileGraph::CompileGraph() {
    auto pair = generateMinimalGraph();
    _begin = std::move(pair.first);
    _end = std::move(pair.second);
}

CompileGraph::CompileGraph(mref<smr<AnchorNode>> begin_, mref<smr<AnchorNode>> end_) noexcept :
    _begin(std::move(begin_)),
    _end(std::move(end_)) {}

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
    const auto result = _expectedProvision.insert(std::move(expected_));
    return result.second;
}

bool CompileGraph::addExpectedRequirement(mref<Requirement> expected_) {
    const auto result = _expectedRequirement.insert(std::move(expected_));
    return result.second;
}

std::span<const Provision> CompileGraph::expectedProvision() const noexcept {
    return std::span(
        _expectedProvision.begin(),
        std::distance(_expectedProvision.begin(), _expectedProvision.end())
    );
}

std::span<const Requirement> CompileGraph::expectedRequirement() const noexcept {
    return std::span(
        _expectedRequirement.begin(),
        std::distance(_expectedRequirement.begin(), _expectedRequirement.end())
    );
}

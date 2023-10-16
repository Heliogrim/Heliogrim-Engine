#include "GraphStager.hpp"

#include <cassert>
#include <Engine.GFX.RenderGraph/Node/Node.hpp>

using namespace hg::engine::gfx::render::pipeline;
using namespace hg::engine::gfx::render;
using namespace hg;

GraphStager::GraphStager() noexcept = default;

GraphStager::~GraphStager() noexcept = default;

decltype(GraphStager::_knownDependencies) GraphStager::extractKnownDependencies() noexcept {
    return _STD move(_knownDependencies);
}

void GraphStager::storeKnownDependencies(mref<decltype(_knownDependencies)> dependencies_) {
    _knownDependencies = _STD move(dependencies_);
}

decltype(GraphStager::_stages) GraphStager::extractStages() noexcept {
    return _STD move(_stages);
}

void GraphStager::stageNode(cref<graph::Node> node_) {

    if (_staged.contains(&node_)) {
        assert(false);
        return;
    }

    const auto knownDep = _knownDependencies.find(&node_);
    if (knownDep != _knownDependencies.end()) {

        /* Validate dependencies and subject on subsequential stages */

        const auto& dependencies = knownDep->second;
        for (const auto& localStaged : _stages.back()) {
            if (dependencies.contains(localStaged.get())) {
                _deferred.push_back(&node_);
                return;
            }
        }
    }

    if (_stages.empty()) {
        __debugbreak();
    }

    _stages.back().emplace_back(&node_);
    _staged.insert(&node_);

    node_.traverse(*this);
}

void GraphStager::operator()(cref<graph::Node> node_) {

    if (not _backlog.empty()) {
        _backlog.push(&node_);
        return;
    }

    // Initial Staging
    _backlog.push(&node_);
    //stageNode(node_);

    do {
        _stages.emplace_back();

        while (not _backlog.empty()) {
            auto node = _backlog.front();
            stageNode(*node);
            _backlog.pop();
        }

        _backlog.push_range(_deferred);
        _deferred.clear();
    } while (not _backlog.empty());
}

void GraphStager::operator()(cref<graph::CompileNode> node_) {
    assert(false);
}

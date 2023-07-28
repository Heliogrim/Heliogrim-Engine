#include "BuilderLayerVisitor.hpp"

#include <Engine.GFX.RenderGraph/Node/Node.hpp>

using namespace hg::engine::gfx::render;
using namespace hg;

BuilderLayerVisitor::~BuilderLayerVisitor() noexcept = default;

void BuilderLayerVisitor::setNodeRcDep(mref<decltype(_nodeRcDep)> map_) {
    _nodeRcDep = _STD move(map_);
}

decltype(BuilderLayerVisitor::_nodeRcDep) BuilderLayerVisitor::extractNodeRcDep() {
    return _STD move(_nodeRcDep);
}

cref<Vector<Vector<nmpt<const graph::Node>>>> BuilderLayerVisitor::layers() const {
    return _layers;
}

bool BuilderLayerVisitor::updateSatisfied(const ptr<const graph::Node> node_) {

    const auto it = _nodeRcDep.find(node_);
    if (it == _nodeRcDep.end() || it->second.empty()) {
        return true;
    }

    // Warning: O(N*M) -> O(N^2)
    for (const auto dep : it->second) {
        if (not _stored.contains(dep)) {
            return false;
        }
    }

    return true;
}

void BuilderLayerVisitor::streamUnwind() {
    while (not _backlog.empty()) {

        auto node = _backlog.front();
        _backlog.pop();

        if (_stored.contains(node)) {
            continue;
        }

        if (not updateSatisfied(node)) {
            _backlog.push(node);
            continue;
        }

        pushAssured(node);
        node->traverse(*this);
    }
}

void BuilderLayerVisitor::assure(u32 layer_) {
    if (_layers.size() <= layer_) {
        _layers.resize(layer_ + 1ui32);
    }
}

void BuilderLayerVisitor::pushAssured(ptr<const graph::Node> node_) {

    const auto it = _nodeRcDep.find(node_);
    if (it != _nodeRcDep.end()) {
        for (const auto layeredNode : _layers[_layer]) {
            if (it->second.contains(layeredNode.get())) {
                ++_layer;
                break;
            }
        }
    }

    assure(_layer);
    _layers[_layer].emplace_back(node_);

    _stored.insert(node_);
}

void BuilderLayerVisitor::operator()(cref<graph::Node> node_) {
    _backlog.push(&node_);

    if (_layer != 0ui32) {
        return;
    }

    streamUnwind();
}

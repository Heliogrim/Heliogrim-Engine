#include "BuilderScanVisitor.hpp"

#include <Engine.GFX.RenderGraph/Node/AnchorNode.hpp>
#include <Engine.GFX.RenderGraph/Node/BarrierNode.hpp>
#include <Engine.GFX.RenderGraph/Node/ConvergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/DivergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Node.hpp>
#include <Engine.GFX.RenderGraph/Node/ProviderNode.hpp>
#include <Engine.GFX.RenderGraph/Node/SelectorNode.hpp>
#include <Engine.GFX.RenderGraph/Node/SubpassNode.hpp>

using namespace hg::engine::gfx::render;
using namespace hg;

BuilderScanVisitor::~BuilderScanVisitor() noexcept = default;

decltype(BuilderScanVisitor::_nodeRc) BuilderScanVisitor::extractNodeRcMap() noexcept {
    return _STD move(_nodeRc);
}

decltype(BuilderScanVisitor::_seen) BuilderScanVisitor::extractSeenSet() noexcept {
    return _STD move(_seen);
}

void BuilderScanVisitor::mark(cref<graph::Node> origin_, cref<graph::Node> node_) {

    auto it = _nodeRc.find(&node_);
    if (it == _nodeRc.end()) {
        const auto result = _nodeRc.insert(_STD make_pair(&node_, DenseSet<ptr<const graph::Node>> {}));
        it = result.first;
    }

    it->second.insert(&origin_);
}

bool BuilderScanVisitor::seen(cref<graph::Node> node_) const noexcept {
    return _seen.contains(&node_);
}

void BuilderScanVisitor::operator()(cref<graph::Node> node_) {
    _seen.insert(&node_);
    node_.traverse(*this);
}

void BuilderScanVisitor::operator()(cref<graph::AnchorNode> node_) {
    if (not seen(node_)) {
        mark(node_, *node_.getNext());
    }
    Visitor::operator()(node_);
}

void BuilderScanVisitor::operator()(cref<graph::BarrierNode> node_) {
    if (not seen(node_)) {
        mark(node_, *node_.getNext());
    }
    Visitor::operator()(node_);
}

void BuilderScanVisitor::operator()(cref<graph::ConvergeNode> node_) {
    if (not seen(node_)) {
        mark(node_, *node_.getNext());
    }
    Visitor::operator()(node_);
}

void BuilderScanVisitor::operator()(cref<graph::DivergeNode> node_) {
    if (not seen(node_)) {
        for (const auto& next : node_.getNext()) {
            mark(node_, *next);
        }
    }
    Visitor::operator()(node_);
}

void BuilderScanVisitor::operator()(cref<graph::SelectorNode> node_) {
    if (not seen(node_)) {
        for (const auto& next : node_.getNext()) {
            mark(node_, *next.node);
        }
    }
    Visitor::operator()(node_);
}

void BuilderScanVisitor::operator()(cref<graph::ProviderNode> node_) {
    if (not seen(node_)) {
        mark(node_, *node_.getNext());
    }
    Visitor::operator()(node_);
}

void BuilderScanVisitor::operator()(cref<graph::SubpassNode> node_) {
    if (not seen(node_)) {
        mark(node_, *node_.getNext());
    }
    Visitor::operator()(node_);
}

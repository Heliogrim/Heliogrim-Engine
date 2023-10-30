#include "GraphScanner.hpp"

#include <Engine.GFX.RenderGraph/Node/Node.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/AnchorNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/BarrierNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/ConvergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/DivergeNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/ProviderNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/SelectorNode.hpp>
#include <Engine.GFX.RenderGraph/Node/Runtime/SubpassNode.hpp>

using namespace hg::engine::gfx::render::pipeline;
using namespace hg::engine::gfx::render;
using namespace hg;

GraphScanner::GraphScanner() noexcept = default;

GraphScanner::~GraphScanner() noexcept = default;

decltype(GraphScanner::_seen) GraphScanner::extractSeenNodes() noexcept {
    return _STD move(_seen);
}

decltype(GraphScanner::_scannedDependencies) GraphScanner::extractNodeDependencies() noexcept {
    return _STD move(_scannedDependencies);
}

void GraphScanner::mark(cref<graph::Node> subject_, cref<graph::Node> dependency_) {
    _scannedDependencies[&subject_].insert(&dependency_);
}

bool GraphScanner::seen(cref<graph::Node> node_) const noexcept {
    return _seen.contains(&node_);
}

void GraphScanner::operator()(cref<graph::Node> node_) {
    _seen.insert(&node_);
    node_.traverse(*this);
}

void GraphScanner::operator()(cref<graph::AnchorNode> node_) {

    if (not seen(node_)) {
        mark(*node_.getNext(), node_);
    }

    Visitor::operator()(node_);
}

void GraphScanner::operator()(cref<graph::BarrierNode> node_) {

    if (not seen(node_)) {
        mark(*node_.getNext(), node_);
    }

    Visitor::operator()(node_);
}

void GraphScanner::operator()(cref<graph::ConvergeNode> node_) {

    if (not seen(node_)) {
        mark(*node_.getNext(), node_);
    }

    Visitor::operator()(node_);
}

void GraphScanner::operator()(cref<graph::DivergeNode> node_) {

    if (not seen(node_)) {
        for (const auto& next : node_.getNext()) {
            mark(*next, node_);
        }
    }

    Visitor::operator()(node_);
}

void GraphScanner::operator()(cref<graph::SelectorNode> node_) {

    if (not seen(node_)) {
        for (const auto& next : node_.getNext()) {
            mark(*next.node, node_);
        }
    }

    Visitor::operator()(node_);
}

void GraphScanner::operator()(cref<graph::ProviderNode> node_) {

    if (not seen(node_)) {
        mark(*node_.getNext(), node_);
    }

    Visitor::operator()(node_);
}

void GraphScanner::operator()(cref<graph::SubpassNode> node_) {

    if (not seen(node_)) {
        mark(*node_.getNext(), node_);
    }

    Visitor::operator()(node_);
}

void GraphScanner::operator()(cref<graph::CompileNode> node_) {
    assert(false);
}

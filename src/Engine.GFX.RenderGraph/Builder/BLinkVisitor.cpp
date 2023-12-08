#include "BLinkVisitor.hpp"

#include <cassert>

#include "Engine.GFX.RenderGraph/Node/Runtime/AnchorNode.hpp"
#include "Engine.GFX.RenderGraph/Node/Runtime/BarrierNode.hpp"
#include "Engine.GFX.RenderGraph/Node/Runtime/ConvergeNode.hpp"
#include "Engine.GFX.RenderGraph/Node/Runtime/DivergeNode.hpp"
#include "Engine.GFX.RenderGraph/Node/Runtime/ProviderNode.hpp"
#include "Engine.GFX.RenderGraph/Node/Runtime/SelectorNode.hpp"
#include "Engine.GFX.RenderGraph/Node/Runtime/SLNode.hpp"
#include "Engine.GFX.RenderGraph/Node/Runtime/SubPassNode.hpp"
#include "Engine.GFX.RenderGraph/Node/Compile/CompileSubPassNode.hpp"

using namespace hg::engine::render::graph;
using namespace hg::engine::render;
using namespace hg;

BLinkVisitor::BLinkVisitor() noexcept = default;

BLinkVisitor::BLinkVisitor(mref<nmpt<const Node>> link_, LinkMode mode_) noexcept :
    Visitor(),
    _linkMode(mode_),
    _linkPredecessor(_STD move(link_)) {}

nmpt<const Node> BLinkVisitor::getLinkPredecessor() const noexcept {
    return _linkPredecessor;
}

void BLinkVisitor::setLinkPredecessor(mref<nmpt<const Node>> link_) {
    _linkPredecessor = _STD move(link_);
}

void BLinkVisitor::operator()(cref<Node> node_) {
    assert(false);
}

void BLinkVisitor::operator()(cref<AnchorNode> node_) {
    const_cast<AnchorNode&>(node_).setPrev(_STD move(_linkPredecessor));
}

void BLinkVisitor::operator()(cref<BarrierNode> node_) {
    const_cast<BarrierNode&>(node_).setPrev(_STD move(_linkPredecessor));
}

void BLinkVisitor::operator()(cref<ConvergeNode> node_) {

    if (_linkMode == LinkMode::eExclusive) {
        const_cast<ConvergeNode&>(node_).setPrev({ _STD move(_linkPredecessor) });
    } else {
        const_cast<ConvergeNode&>(node_).addPrev(_STD move(_linkPredecessor));
    }
}

void BLinkVisitor::operator()(cref<DivergeNode> node_) {
    const_cast<DivergeNode&>(node_).setPrev(_STD move(_linkPredecessor));
}

void BLinkVisitor::operator()(cref<SelectorNode> node_) {
    const_cast<SelectorNode&>(node_).setPrev(_STD move(_linkPredecessor));
}

void BLinkVisitor::operator()(cref<ProviderNode> node_) {
    const_cast<ProviderNode&>(node_).setPrev(_STD move(_linkPredecessor));
}

void BLinkVisitor::operator()(cref<SLNode> node_) {
    const_cast<SLNode&>(node_).setPrev(_STD move(_linkPredecessor));
}

void BLinkVisitor::operator()(cref<SubPassNodeBase> node_) {
    const_cast<SubPassNodeBase&>(node_).setPrev(_STD move(_linkPredecessor));
}

void BLinkVisitor::operator()(cref<CompileNode> node_) {
    assert(false);
}

void BLinkVisitor::operator()(cref<CompileSubPassNode> node_) {
    const_cast<CompileSubPassNode&>(node_).setPrev(_STD move(_linkPredecessor));
}

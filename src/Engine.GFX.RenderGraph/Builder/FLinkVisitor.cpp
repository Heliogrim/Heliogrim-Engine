#include "FLinkVisitor.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>

#include "Engine.GFX.RenderGraph/Node/Runtime/AnchorNode.hpp"
#include "Engine.GFX.RenderGraph/Node/Runtime/BarrierNode.hpp"
#include "Engine.GFX.RenderGraph/Node/Runtime/ConvergeNode.hpp"
#include "Engine.GFX.RenderGraph/Node/Runtime/DivergeNode.hpp"
#include "Engine.GFX.RenderGraph/Node/Runtime/ProviderNode.hpp"
#include "Engine.GFX.RenderGraph/Node/Runtime/SelectorNode.hpp"
#include "Engine.GFX.RenderGraph/Node/Runtime/SLNode.hpp"
#include "Engine.GFX.RenderGraph/Node/Runtime/SubpassNode.hpp"
#include "Engine.GFX.RenderGraph/Node/Compile/CompileSubPassNode.hpp"

using namespace hg::engine::render::graph;
using namespace hg::engine::render;
using namespace hg;

FLinkVisitor::FLinkVisitor() noexcept = default;

FLinkVisitor::FLinkVisitor(mref<smr<Node>> link_, LinkMode mode_) noexcept :
    Visitor(),
    _linkMode(mode_),
    _linkSuccessor(std::move(link_)) {}

smr<Node> FLinkVisitor::getLinkSuccessor() const {
    return clone(_linkSuccessor);
}

void FLinkVisitor::setLinkSuccessor(mref<smr<Node>> link_) {
    _linkSuccessor = std::move(link_);
}

void FLinkVisitor::operator()(cref<Node> node_) {
    assert(false);
}

void FLinkVisitor::operator()(cref<AnchorNode> node_) {
    const_cast<AnchorNode&>(node_).setNext(std::move(_linkSuccessor));
}

void FLinkVisitor::operator()(cref<BarrierNode> node_) {
    const_cast<BarrierNode&>(node_).setNext(std::move(_linkSuccessor));
}

void FLinkVisitor::operator()(cref<ConvergeNode> node_) {
    const_cast<ConvergeNode&>(node_).setNext(std::move(_linkSuccessor));
}

void FLinkVisitor::operator()(cref<DivergeNode> node_) {

    if (_linkMode == LinkMode::eExclusive) {
        const_cast<DivergeNode&>(node_).setNext({ std::move(_linkSuccessor) });
    } else {
        const_cast<DivergeNode&>(node_).addNext(std::move(_linkSuccessor));
    }
}

void FLinkVisitor::operator()(cref<SelectorNode> node_) {

    if (_linkMode == LinkMode::eExclusive) {
        assert(false);
    } else {
        const_cast<SelectorNode&>(node_).addNext(true, std::move(_linkSuccessor));
    }
}

void FLinkVisitor::operator()(cref<ProviderNode> node_) {
    const_cast<ProviderNode&>(node_).setNext(std::move(_linkSuccessor));
}

void FLinkVisitor::operator()(cref<SLNode> node_) {
    const_cast<SLNode&>(node_).setNext(std::move(_linkSuccessor));
}

void FLinkVisitor::operator()(cref<SubPassNodeBase> node_) {
    const_cast<SubPassNodeBase&>(node_).setNext(std::move(_linkSuccessor));
}

void FLinkVisitor::operator()(cref<CompileNode> node_) {
    assert(false);
}

void FLinkVisitor::operator()(cref<CompileSubPassNode> node_) {
    const_cast<CompileSubPassNode&>(node_).setNext(std::move(_linkSuccessor));
}

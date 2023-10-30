#include "ProviderNode.hpp"

#include <Engine.Common/Make.hpp>

#include "../../Component/ProviderComponent.hpp"
#include "../../Visitor/Visitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

ProviderNode::ProviderNode() noexcept :
    RuntimeNode(make_smr<Auxiliary>()) {
    _auxiliary->add(uptr<ProviderComponent> { new ProviderComponent() });
}

void ProviderNode::accept(ref<Visitor> visitor_) const {
    visitor_(*this);
}

void ProviderNode::traverse(ref<Visitor> visitor_) const {
    _next->accept(visitor_);
}

void ProviderNode::rtraverse(ref<Visitor> visitor_) const {
    _prev->accept(visitor_);
}

void ProviderNode::setNext(mref<smr<Node>> next_) {
    _next = _STD move(next_);
}

smr<Node> ProviderNode::getNext() const noexcept {
    return _next;
}

void ProviderNode::setPrev(mref<nmpt<const Node>> prev_) {
    _prev = _STD move(prev_);
}

nmpt<const Node> ProviderNode::getPrev() const noexcept {
    return _prev;
}

nmpt<const ProviderComponent> ProviderNode::getProviderComponent() const noexcept {
    return const_cast<ptr<this_type>>(this)->getProviderComponent();
}

nmpt<ProviderComponent> ProviderNode::getProviderComponent() {
    return _auxiliary->getFirstOfType<ProviderComponent>();
}

IterationResult ProviderNode::iterate(cref<IterationPassContext> ctx_) noexcept {
    return IterationResultBits::eNone;
}

IterationResult ProviderNode::resolve(cref<ResolvePassContext> ctx_) noexcept {
    return IterationResultBits::eNone;
}

IterationResult ProviderNode::execute(cref<ExecutionPassContext> ctx_) noexcept {
    return IterationResultBits::eNone;
}

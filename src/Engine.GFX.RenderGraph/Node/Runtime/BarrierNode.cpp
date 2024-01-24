#include "BarrierNode.hpp"

#include <Engine.Common/Make.hpp>

#include "../../Component/BarrierComponent.hpp"
#include "../../Visitor/Visitor.hpp"

using namespace hg::engine::render::graph;
using namespace hg;

BarrierNode::BarrierNode() noexcept :
    RuntimeNode(make_smr<Auxiliary>()) {
    _auxiliary->add(uptr<BarrierComponent> { new BarrierComponent() });
}

void BarrierNode::accept(ref<Visitor> visitor_) const {
    visitor_(*this);
}

void BarrierNode::traverse(ref<Visitor> visitor_) const {
    _next->accept(visitor_);
}

void BarrierNode::rtraverse(ref<Visitor> visitor_) const {
    _prev->accept(visitor_);
}

void BarrierNode::setNext(mref<smr<Node>> next_) {
    _next = std::move(next_);
}

smr<Node> BarrierNode::getNext() const noexcept {
    return _next;
}

void BarrierNode::setPrev(mref<nmpt<const Node>> prev_) {
    _prev = std::move(prev_);
}

nmpt<const Node> BarrierNode::getPrev() const noexcept {
    return _prev;
}

nmpt<const BarrierComponent> BarrierNode::getBarrierComponent() const noexcept {
    return const_cast<ptr<this_type>>(this)->getBarrierComponent();
}

nmpt<BarrierComponent> BarrierNode::getBarrierComponent() {
    const auto it = std::ranges::find_if(
        _auxiliary->components(),
        [](const ptr<Component> component_) {
            return component_->getMetaClass()->exact<BarrierComponent>();
        }
    );
    return it != _auxiliary->components().end() ?
               static_cast<ptr<BarrierComponent>>(*it) :
               nmpt<BarrierComponent> {};
}

IterationResult BarrierNode::iterate(cref<IterationPassContext> ctx_) noexcept {
    return IterationResultBits::eNone;
}

IterationResult BarrierNode::resolve(cref<ResolvePassContext> ctx_) noexcept {
    return IterationResultBits::eNone;
}

IterationResult BarrierNode::execute(cref<ExecutionPassContext> ctx_) noexcept {
    return IterationResultBits::eNone;
}

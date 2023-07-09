#include "BarrierNode.hpp"

#include <Engine.Common/Make.hpp>

#include "../Component/BarrierComponent.hpp"
#include "../Visitor/Visitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

BarrierNode::BarrierNode() noexcept :
    Node(
        make_smr<Auxiliary>()
    ) {
    _auxiliary->add(uptr<BarrierComponent> { HeliogrimObject::create<BarrierComponent>() });
}

void BarrierNode::accept(ref<Visitor> visitor_) const {
    visitor_(*this);
}

void BarrierNode::traverse(ref<Visitor> visitor_) const {
    _next->accept(visitor_);
}

void BarrierNode::setNext(mref<smr<Node>> next_) {
    _next = _STD move(next_);
}

smr<Node> BarrierNode::getNext() const noexcept {
    return _next;
}

nmpt<const BarrierComponent> BarrierNode::getBarrierComponent() const noexcept {
    return const_cast<ptr<this_type>>(this)->getBarrierComponent();
}

nmpt<BarrierComponent> BarrierNode::getBarrierComponent() {
    const auto it = _STD ranges::find_if(
        _auxiliary->components(),
        [](const ptr<Component> component_) {
            return component_->getClass()->isExactType<BarrierComponent>();
        }
    );
    return it != _auxiliary->components().end() ?
               static_cast<ptr<BarrierComponent>>(*it) :
               nmpt<BarrierComponent> {};
}

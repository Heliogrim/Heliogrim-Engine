#include "ProviderNode.hpp"

#include <Engine.Common/Make.hpp>

#include "../Component/ProviderComponent.hpp"
#include "../Visitor/Visitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

ProviderNode::ProviderNode() noexcept :
    Node(make_smr<Auxiliary>()) {
    _auxiliary->add(uptr<ProviderComponent> { new ProviderComponent() });
}

void ProviderNode::accept(ref<Visitor> visitor_) const {
    visitor_(*this);
}

void ProviderNode::traverse(ref<Visitor> visitor_) const {
    _next->accept(visitor_);
}

bool ProviderNode::empty() const noexcept {
    return _next.empty();
}

void ProviderNode::setNext(mref<smr<Node>> next_) {
    _next = _STD move(next_);
}

smr<Node> ProviderNode::getNext() const noexcept {
    return _next;
}

nmpt<const ProviderComponent> ProviderNode::getProviderComponent() const noexcept {
    return const_cast<ptr<this_type>>(this)->getProviderComponent();
}

nmpt<ProviderComponent> ProviderNode::getProviderComponent() {
    const auto it = _STD ranges::find_if(
        _auxiliary->components(),
        [](cref<ptr<Component>> component_) {
            return component_->getMetaClass()->exact<ProviderComponent>();
        }
    );
    return it != _auxiliary->components().end() ?
               static_cast<ptr<ProviderComponent>>(*it) :
               nmpt<ProviderComponent> {};
}

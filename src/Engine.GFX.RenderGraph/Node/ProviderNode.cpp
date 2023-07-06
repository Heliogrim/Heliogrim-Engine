#include "ProviderNode.hpp"

#include "../Component/ProviderComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

ProviderNode::ProviderNode() noexcept :
    Node(make_smr<Auxiliary>()) {
    _auxiliary->add(uptr<ProviderComponent> { HeliogrimObject::create<ProviderComponent>() });
}

void ProviderNode::traverse(ref<Visitor> visitor_) {
    _next->accept(visitor_);
}

bool ProviderNode::empty() const noexcept {
    return _next.empty();
}

void ProviderNode::setNext(mref<smr<Node>> next_) {
    _next = _STD move(next_);
}

nmpt<ProviderComponent> ProviderNode::getProviderComponent() {
    const auto it = _STD ranges::find_if(
        _auxiliary->components(),
        [](cref<ptr<Component>> component_) {
            return component_->getClass()->isExactType<ProviderComponent>();
        }
    );
    return it != _auxiliary->components().end() ?
               static_cast<ptr<ProviderComponent>>(*it) :
               nmpt<ProviderComponent> {};
}

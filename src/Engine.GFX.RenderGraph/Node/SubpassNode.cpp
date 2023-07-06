#include "SubpassNode.hpp"

#include "../Component/SubpassComponent.hpp"
#include "../Component/SubpassMultiAccelComponent.hpp"
#include "../Component/SubpassSingleAccelComponent.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

SubpassNode::SubpassNode(SubpassAccelMode mode_) noexcept :
    Node(
        make_smr<Auxiliary>()
    ) {
    _auxiliary->add(uptr<SubpassComponent> { HeliogrimObject::create<SubpassComponent>() });

    if (mode_ == SubpassAccelMode::eSingle) {
        _auxiliary->add(uptr<SubpassSingleAccelComponent> { HeliogrimObject::create<SubpassSingleAccelComponent>() });
    } else {
        _auxiliary->add(uptr<SubpassMultiAccelComponent> { HeliogrimObject::create<SubpassMultiAccelComponent>() });
    }
}

SubpassNode::~SubpassNode() noexcept = default;

void SubpassNode::traverse(ref<Visitor> visitor_) {
    _next->accept(visitor_);
}

void SubpassNode::setNext(mref<smr<Node>> next_) {
    _next = _STD move(next_);
}

nmpt<SubpassComponent> SubpassNode::getSubpassComponent() const noexcept {
    const auto it = _STD ranges::find_if(
        _auxiliary->components(),
        [](const ptr<Component> component_) {
            return component_->getClass()->isExactType<SubpassComponent>();
        }
    );
    return it != _auxiliary->components().end() ?
               static_cast<ptr<SubpassComponent>>(*it) :
               nmpt<SubpassComponent> {};
}

nmpt<SubpassAccelComponent> SubpassNode::getSubpassAcceleration() const noexcept {
    const auto it = _STD ranges::find_if(
        _auxiliary->components(),
        [](const ptr<Component> component_) {
            return component_->getClass()->isExactType<SubpassSingleAccelComponent>() ||
                component_->getClass()->isExactType<SubpassMultiAccelComponent>();
        }
    );
    return it != _auxiliary->components().end() ?
               static_cast<ptr<SubpassAccelComponent>>(*it) :
               nmpt<SubpassAccelComponent> {};
}

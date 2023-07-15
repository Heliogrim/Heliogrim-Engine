#include "SubpassNode.hpp"

#include "../Component/SubpassComponent.hpp"
#include "../Component/Subpass/SubpassMaterialAccelComponent.hpp"
#include "../Component/Subpass/SubpassMultiAccelComponent.hpp"
#include "../Component/Subpass/SubpassSingleAccelComponent.hpp"
#include "../Visitor/Visitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

SubpassNode::SubpassNode(SubpassAccelMode mode_) noexcept :
    Node(
        make_smr<Auxiliary>()
    ) {
    _auxiliary->add(uptr<SubpassComponent> { HeliogrimObject::create<SubpassComponent>() });

    if (mode_ == SubpassAccelMode::eSingle) {
        _auxiliary->add(
            uptr<SubpassSingleAccelComponent> { HeliogrimObject::create<SubpassSingleAccelComponent>() }
        );
    } else if (mode_ == SubpassAccelMode::eMulti) {
        _auxiliary->add(
            uptr<SubpassMultiAccelComponent> { HeliogrimObject::create<SubpassMultiAccelComponent>() }
        );
    } else if (mode_ == SubpassAccelMode::eMaterial) {
        _auxiliary->add(
            uptr<SubpassMaterialAccelComponent> { HeliogrimObject::create<SubpassMaterialAccelComponent>() }
        );
    }
}

SubpassNode::~SubpassNode() noexcept = default;

void SubpassNode::accept(ref<Visitor> visitor_) const {
    visitor_(*this);
}

void SubpassNode::traverse(ref<Visitor> visitor_) const {
    _next->accept(visitor_);
}

void SubpassNode::setNext(mref<smr<Node>> next_) {
    _next = _STD move(next_);
}

smr<Node> SubpassNode::getNext() const noexcept {
    return _next;
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

#include "SubpassNode.hpp"

#include <Engine.Common/Make.hpp>

#include "../Component/SubpassComponent.hpp"
#include "../Component/Subpass/SubpassMaterialAccelComponent.hpp"
#include "../Component/Subpass/SubpassMultiAccelComponent.hpp"
#include "../Component/Subpass/SubpassSceneMeshInvocationComponent.hpp"
#include "../Component/Subpass/SubpassSingleAccelComponent.hpp"
#include "../Visitor/Visitor.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

SubpassNode::SubpassNode(SubpassAccelMode mode_) noexcept :
    Node(
        make_smr<Auxiliary>()
    ) {
    auxAdd(make_uptr<SubpassComponent>());

    if (mode_ == SubpassAccelMode::eSingle) {
        auxAdd(make_uptr<SubpassSingleAccelComponent>());
    } else if (mode_ == SubpassAccelMode::eMulti) {
        auxAdd(make_uptr<SubpassMultiAccelComponent>());
    } else if (mode_ == SubpassAccelMode::eMaterial) {
        auxAdd(make_uptr<SubpassMaterialAccelComponent>());
    }
}

SubpassNode::~SubpassNode() noexcept = default;

void SubpassNode::accept(ref<Visitor> visitor_) const {
    visitor_(*this);
}

void SubpassNode::traverse(ref<Visitor> visitor_) const {
    _next->accept(visitor_);
}

void SubpassNode::rtraverse(ref<Visitor> visitor_) const {
    _prev->accept(visitor_);
}

void SubpassNode::setNext(mref<smr<Node>> next_) {
    _next = _STD move(next_);
}

smr<Node> SubpassNode::getNext() const noexcept {
    return _next;
}

void SubpassNode::setPrev(mref<smr<Node>> prev_) {
    _prev = _STD move(prev_);
}

smr<Node> SubpassNode::getPrev() const noexcept {
    return _prev;
}

nmpt<SubpassComponent> SubpassNode::getSubpassComponent() const noexcept {
    const auto it = _STD ranges::find_if(
        _auxiliary->components(),
        [](const ptr<Component> component_) {
            return component_->getMetaClass()->exact<SubpassComponent>();
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
            return component_->getMetaClass()->is<SubpassAccelComponent>();
        }
    );
    return it != _auxiliary->components().end() ?
               static_cast<ptr<SubpassAccelComponent>>(*it) :
               nmpt<SubpassAccelComponent> {};
}

nmpt<SubpassInvocationComponent> SubpassNode::getSubpassInvocation() const noexcept {
    const auto it = _STD ranges::find_if(
        _auxiliary->components(),
        [](const ptr<Component> component_) {
            return component_->getMetaClass()->exact<SubpassSceneMeshInvocationComponent>();
        }
    );
    return it != _auxiliary->components().end() ?
               static_cast<ptr<SubpassInvocationComponent>>(*it) :
               nmpt<SubpassInvocationComponent> {};
}

void SubpassNode::setSubpassInvocation(mref<uptr<SubpassInvocationComponent>> invocationComponent_) {

    auto prev = getSubpassInvocation();
    if (prev != nullptr) {
        _auxiliary->remove(prev.get());
    }

    /**/

    _auxiliary->add(_STD move(invocationComponent_));
}

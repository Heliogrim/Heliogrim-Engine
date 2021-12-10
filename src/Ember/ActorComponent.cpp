#include "ActorComponent.hpp"

using namespace ember;

ActorComponent::ActorComponent(mref<component_type_id> typeId_, mref<CachedActorPointer> owner_,
    mref<ptr<ActorComponent>> parent_) :
    LogicComponent(_STD move(typeId_)),
    _owner(_STD move(owner_)),
    _parent(_STD move(parent_)) {}

ActorComponent::ActorComponent(mref<CachedActorPointer> owner_, mref<ptr<ActorComponent>> parent_) :
    LogicComponent(component_type_id { type_id }),
    _owner(_STD move(owner_)),
    _parent(_STD move(parent_)) {}

ptr<Actor> ActorComponent::getOwner() const noexcept {
    return _owner.cached;
}

ptr<Actor> ActorComponent::getRootActor() const noexcept {
    const auto root { getRootComponent() };
    return root ? root->getOwner() : nullptr;
}

ptr<ActorComponent> ActorComponent::getRootComponent() const noexcept {

    if (_parent) {
        return _parent->getRootComponent();
    }

    return nullptr;
}

ptr<ActorComponent> ActorComponent::getParentComponent() const noexcept {
    return _parent;
}

cref<math::Bounding> ActorComponent::getBoundaries() const noexcept {
    return _boundaries;
}

Transform ActorComponent::getLocalTransform() const noexcept {
    return Transform {
        _localPosition,
        _localRotation,
        _localScale
    };
}

cref<Transform> ActorComponent::getWorldTransform() const noexcept {
    return _worldTransform;
}

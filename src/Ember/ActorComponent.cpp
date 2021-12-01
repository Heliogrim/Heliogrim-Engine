#include "ActorComponent.hpp"

using namespace ember;

ptr<Actor> ActorComponent::getRootActor() const noexcept {
    return nullptr;
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

#include "ActorComponent.hpp"

using namespace ember;

ptr<Actor> ActorComponent::getRootActor() const noexcept {
    return nullptr;
}

ptr<ActorComponent> ActorComponent::getRootComponent() const noexcept {
    return nullptr;
}

ptr<ActorComponent> ActorComponent::getParentComponent() const noexcept {
    return nullptr;
}

cref<math::Bounding> ActorComponent::getBoundaries() const noexcept {
    return math::Bounding {};
}

cref<math::Transformation> ActorComponent::getWorldTransform() const noexcept {
    return math::ZeroTransformation {};
}

cref<math::Transformation> ActorComponent::getLocalTransform() const noexcept {
    return math::ZeroTransformation {};
}

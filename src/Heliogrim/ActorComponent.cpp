#include "ActorComponent.hpp"

using namespace hg;

ActorComponent::ActorComponent(
    mref<component_type_id> typeId_,
    mref<CachedActorPointer> owner_,
    mref<ptr<ActorComponent>> parent_
) :
    LogicComponent(_STD move(typeId_)),
    _owner(_STD move(owner_)),
    _parent(_STD move(parent_)),
    _worldTransform({}, {}, math::vec3 { 1.F }) {}

ActorComponent::ActorComponent(mref<CachedActorPointer> owner_, mref<ptr<ActorComponent>> parent_) :
    LogicComponent(component_type_id { typeId }),
    _owner(_STD move(owner_)),
    _parent(_STD move(parent_)),
    _worldTransform({}, {}, math::vec3 { 1.F }) {}

ptr<Actor> ActorComponent::getOwner() const noexcept {
    return _owner.cached;
}

ptr<Actor> ActorComponent::getRootActor() const noexcept {
    const auto root { getRootComponent() };
    return root ? root->getOwner() : nullptr;
}

ptr<ActorComponent> ActorComponent::getRootComponent() const noexcept {

    if (not _parent) {
        return const_cast<ptr<ActorComponent>>(this);
    }

    return _parent->getRootComponent();
}

ptr<ActorComponent> ActorComponent::getParentComponent() const noexcept {
    return _parent;
}

cref<math::Bounding> ActorComponent::getBoundaries() const noexcept {
    return _boundaries;
}

cref<Transform> ActorComponent::getLocalTransform() const noexcept {
    return _localTransform;
}

ref<Transform> ActorComponent::getLocalTransform() noexcept {
    return _localTransform;
}

cref<Transform> ActorComponent::getWorldTransform() const noexcept {
    return _worldTransform;
}

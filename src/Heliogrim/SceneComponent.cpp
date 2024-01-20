#include "SceneComponent.hpp"

using namespace hg;

SceneComponent::SceneComponent(
    mref<component_type_id> typeId_,
    mref<CachedActorPointer> owner_,
    mref<ptr<ActorComponent>> parent_
) :
    InheritMeta(_STD move(typeId_), _STD move(owner_), _STD move(parent_)) {}

SceneComponent::~SceneComponent() {
    ActorComponent::~ActorComponent();
}

#include "SkeletalGeometryComponent.hpp"

using namespace ember;

SkeletalGeometryComponent::SkeletalGeometryComponent(
    mref<CachedActorPointer> owner_,
    mref<ptr<ActorComponent>> parent_
) :
    ModelComponent(component_type_id { typeId }, _STD move(owner_), _STD move(parent_)) {}

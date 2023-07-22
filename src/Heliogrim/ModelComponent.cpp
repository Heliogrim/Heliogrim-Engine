#include "ModelComponent.hpp"

using namespace hg;

ModelComponent::ModelComponent(
    mref<component_type_id> typeId_,
    mref<CachedActorPointer> owner_,
    mref<ptr<ActorComponent>> parent_
) :
    InheritMeta(_STD move(typeId_), _STD move(owner_), _STD move(parent_)) {}

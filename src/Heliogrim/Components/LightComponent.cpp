#include "../LightComponent.hpp"

using namespace hg;

LightComponent::LightComponent(
    mref<component_type_id> typeId_,
    mref<CachedActorPointer> owner_,
    mref<ptr<ActorComponent>> parent_
) :
    InheritMeta(std::move(typeId_), std::move(owner_), std::move(parent_)) {}

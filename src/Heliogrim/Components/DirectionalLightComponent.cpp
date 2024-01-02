#include "DirectionalLightComponent.hpp"

using namespace hg;

DirectionalLightComponent::DirectionalLightComponent(
    mref<CachedActorPointer> owner_,
    mref<ptr<ActorComponent>> parent_
) :
    InheritMeta(component_type_id { typeId }, _STD move(owner_), _STD move(parent_)) {}

DirectionalLightComponent::~DirectionalLightComponent() = default;

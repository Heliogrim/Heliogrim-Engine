#include "PointLightComponent.hpp"

using namespace hg;

PointLightComponent::PointLightComponent(
    mref<CachedActorPointer> owner_,
    mref<ptr<ActorComponent>> parent_
) :
    InheritMeta(component_type_id { typeId }, _STD move(owner_), _STD move(parent_)) {}

PointLightComponent::~PointLightComponent() = default;

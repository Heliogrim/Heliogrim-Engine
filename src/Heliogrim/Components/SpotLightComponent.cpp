#include "SpotLightComponent.hpp"

using namespace hg;

SpotLightComponent::SpotLightComponent(
    mref<CachedActorPointer> owner_,
    mref<ptr<ActorComponent>> parent_
) :
    InheritMeta(component_type_id { typeId }, _STD move(owner_), _STD move(parent_)),
    _innerCone(45.F),
    _outerCone(60.F),
    _distanceLimit(0.F) {}

SpotLightComponent::~SpotLightComponent() = default;

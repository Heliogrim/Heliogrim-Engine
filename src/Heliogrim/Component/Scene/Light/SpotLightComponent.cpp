#include "SpotLightComponent.hpp"

using namespace hg;

SpotLightComponent::SpotLightComponent(
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(component_type_id { typeId }, std::move(owner_), std::move(parent_)),
	_innerCone(45.F),
	_outerCone(60.F),
	_distanceLimit(0.F) {}

SpotLightComponent::~SpotLightComponent() = default;

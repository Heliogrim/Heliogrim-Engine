#include "PointLightComponent.hpp"

using namespace hg;

PointLightComponent::PointLightComponent(
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(component_type_id { typeId }, std::move(owner_), std::move(parent_)),
	_distanceLimit(0.F),
	_luminance() {}

PointLightComponent::~PointLightComponent() = default;

#include "PointLightComponent.hpp"

#include <Engine.Common/Move.hpp>

using namespace hg;

PointLightComponent::PointLightComponent(
	mref<ComponentGuid> guid_,
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(::hg::move(guid_), ComponentTypeId { typeId }, std::move(owner_), std::move(parent_)),
	_distanceLimit(0.F),
	_luminance() {}

PointLightComponent::~PointLightComponent() = default;
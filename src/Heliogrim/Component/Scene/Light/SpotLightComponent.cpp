#include "SpotLightComponent.hpp"

#include <Engine.Common/Move.hpp>

using namespace hg;

SpotLightComponent::SpotLightComponent(
	mref<ComponentGuid> guid_,
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(::hg::move(guid_), ComponentTypeId { typeId }, std::move(owner_), std::move(parent_)),
	_innerCone(45.F),
	_outerCone(60.F),
	_distanceLimit(0.F) {}

SpotLightComponent::~SpotLightComponent() = default;
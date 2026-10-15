#include "SphereShapeCollisionComponent.hpp"

#include <Engine.Common/Move.hpp>

using namespace hg;

SphereShapeCollisionComponent::SphereShapeCollisionComponent(
	mref<ComponentGuid> guid_,
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(::hg::move(guid_), ComponentTypeId { typeId }, ::hg::move(owner_), ::hg::move(parent_)),
	settings({ .isStatic = true, .isTrigger = false }),
	radius({ 1.f }) {}

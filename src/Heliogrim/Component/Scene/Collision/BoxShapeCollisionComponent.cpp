#include "BoxShapeCollisionComponent.hpp"

#include <Engine.Common/Move.hpp>
#include <Engine.Common/Math/Coordinates.hpp>

using namespace hg;

BoxShapeCollisionComponent::BoxShapeCollisionComponent(
	mref<ComponentGuid> guid_,
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(::hg::move(guid_), ComponentTypeId { typeId }, ::hg::move(owner_), ::hg::move(parent_)),
	settings({ .isStatic = true, .isTrigger = false }),
	extent(math::vec3_one) {}

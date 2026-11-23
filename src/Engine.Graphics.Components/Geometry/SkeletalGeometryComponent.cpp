#include "SkeletalGeometryComponent.hpp"

using namespace hg;

SkeletalGeometryComponent::SkeletalGeometryComponent(
	mref<ComponentGuid> guid_,
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(std::move(guid_), ComponentTypeId { typeId }, std::move(owner_), std::move(parent_)) {}
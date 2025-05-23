#include "SkeletalGeometryComponent.hpp"

using namespace hg;

SkeletalGeometryComponent::SkeletalGeometryComponent(
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(component_type_id { typeId }, std::move(owner_), std::move(parent_)) {}

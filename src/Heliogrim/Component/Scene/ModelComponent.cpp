#include "ModelComponent.hpp"

using namespace hg;

ModelComponent::ModelComponent(
	mref<component_type_id> typeId_,
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(std::move(typeId_), std::move(owner_), std::move(parent_)) {}

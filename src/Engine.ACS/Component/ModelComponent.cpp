#include "ModelComponent.hpp"

using namespace hg;

ModelComponent::ModelComponent(
	mref<ComponentGuid> guid_,
	mref<ComponentTypeId> typeId_,
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(std::move(guid_), std::move(typeId_), std::move(owner_), std::move(parent_)) {}
#include "LightComponent.hpp"

#include <Engine.Common/Move.hpp>

using namespace hg;

LightComponent::LightComponent(
	mref<ComponentGuid> guid_,
	mref<ComponentTypeId> typeId_,
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(::hg::move(guid_), std::move(typeId_), std::move(owner_), std::move(parent_)) {}
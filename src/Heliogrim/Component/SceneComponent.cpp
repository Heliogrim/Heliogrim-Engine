#include "SceneComponent.hpp"

#include <Engine.Common/__macro.hpp>

import Engine.Common;

using namespace hg;

SceneComponent::SceneComponent(
	mref<ComponentGuid> guid_,
	mref<ComponentTypeId> typeId_,
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(::hg::move(guid_), ::hg::move(typeId_), ::hg::move(owner_), ::hg::move(parent_)) {}

SceneComponent::~SceneComponent() {
	HierarchyComponent::~HierarchyComponent();
}
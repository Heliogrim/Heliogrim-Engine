#include "SceneComponent.hpp"

using namespace hg;

SceneComponent::SceneComponent(
	mref<component_type_id> typeId_,
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(std::move(typeId_), std::move(owner_), std::move(parent_)) {}

SceneComponent::~SceneComponent() {
	HierarchyComponent::~HierarchyComponent();
}

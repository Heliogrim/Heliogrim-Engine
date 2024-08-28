#include "DirectionalLightComponent.hpp"

using namespace hg;

DirectionalLightComponent::DirectionalLightComponent(
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(component_type_id { typeId }, std::move(owner_), std::move(parent_)) {}

DirectionalLightComponent::~DirectionalLightComponent() = default;

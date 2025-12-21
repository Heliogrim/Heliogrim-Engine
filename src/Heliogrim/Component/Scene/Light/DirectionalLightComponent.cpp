#include "DirectionalLightComponent.hpp"

#include <Engine.Common/Move.hpp>

using namespace hg;

DirectionalLightComponent::DirectionalLightComponent(
	mref<ComponentGuid> guid_,
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(::hg::move(guid_), ComponentTypeId { typeId }, std::move(owner_), std::move(parent_)) {}

DirectionalLightComponent::~DirectionalLightComponent() = default;
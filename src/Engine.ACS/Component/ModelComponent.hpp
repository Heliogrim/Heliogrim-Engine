#pragma once

#include "../SceneComponent.hpp"

namespace hg {
	class ModelComponent :
		public InheritMeta<ModelComponent, SceneComponent> {
	public:
		using this_type = ModelComponent;
		using underlying_type = SceneComponent;

	public:
		ModelComponent(
			mref<ComponentGuid> guid_,
			mref<ComponentTypeId> typeId_,
			mref<CachedActorPointer> owner_,
			mref<ptr<HierarchyComponent>> parent_
		);

		~ModelComponent() override = default;
	};
}
#pragma once

#include "HierarchyComponent.hpp"

namespace hg {
	struct SceneNodeModel;
}

namespace hg {
	class SceneComponent :
		public InheritMeta<SceneComponent, HierarchyComponent> {
	public:
		using this_type = SceneComponent;
		using underlying_type = HierarchyComponent;

	protected:
		SceneComponent(
			mref<ComponentGuid> guid_,
			mref<ComponentTypeId> typeId_,
			mref<CachedActorPointer> owner_,
			mref<ptr<HierarchyComponent>> parent_
		);

	public:
		~SceneComponent() override;
	};
}
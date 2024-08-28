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

	public:
		SceneComponent(
			mref<component_type_id> typeId_,
			mref<CachedActorPointer> owner_,
			mref<ptr<HierarchyComponent>> parent_
		);

		~SceneComponent() override;
	};
}

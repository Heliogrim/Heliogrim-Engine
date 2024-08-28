#pragma once

#include "../ModelComponent.hpp"

namespace hg {
	class SkeletalGeometryComponent :
		public InheritMeta<SkeletalGeometryComponent, ModelComponent> {
	public:
		using this_type = SkeletalGeometryComponent;
		using underlying_type = ModelComponent;

		constexpr static component_type_id typeId { "SkeletalGeometryComponent"_typeId };

	public:
		SkeletalGeometryComponent(mref<CachedActorPointer> owner_, mref<ptr<HierarchyComponent>> parent_);

		~SkeletalGeometryComponent() override = default;
	};
}

#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
#endif

namespace hg {
	class SkeletalGeometryComponent :
		public InheritMeta<SkeletalGeometryComponent, ModelComponent> {
	public:
		using this_type = SkeletalGeometryComponent;
		using underlying_type = ModelComponent;

		constexpr static ComponentTypeId typeId { "SkeletalGeometryComponent"_typeId };

	public:
		SkeletalGeometryComponent(mref<ComponentGuid> guid_, mref<CachedActorPointer> owner_, mref<ptr<HierarchyComponent>> parent_);

		~SkeletalGeometryComponent() override = default;
	};
}
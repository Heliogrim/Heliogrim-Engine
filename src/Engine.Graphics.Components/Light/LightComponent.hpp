#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
#endif

namespace hg {
	class LightComponent :
		public InheritMeta<LightComponent, SceneComponent> {
	public:
		using this_type = LightComponent;
		using underlying_type = SceneComponent;

	public:
		LightComponent(
			mref<ComponentGuid> guid_,
			mref<ComponentTypeId> typeId_,
			mref<CachedActorPointer> owner_,
			mref<ptr<HierarchyComponent>> parent_
		);
	};
}
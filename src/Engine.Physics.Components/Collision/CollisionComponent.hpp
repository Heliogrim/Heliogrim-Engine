#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.ACS;
#endif

namespace hg {
	class CollisionComponent :
		public InheritMeta<CollisionComponent, SceneComponent> {
	public:
		using this_type = CollisionComponent;
		using underlying_type = SceneComponent;

	public:
		using InheritMeta::InheritMeta;
	};
}

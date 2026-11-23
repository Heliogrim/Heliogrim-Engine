#pragma once

#include "CollisionComponent.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Reflect;
#endif

namespace hg {
	class ShapeCollisionComponent :
		public InheritMeta<ShapeCollisionComponent, CollisionComponent> {
	public:
		using this_type = ShapeCollisionComponent;
		using underlying_type = CollisionComponent;

	public:
		using InheritMeta::InheritMeta;
	};
}

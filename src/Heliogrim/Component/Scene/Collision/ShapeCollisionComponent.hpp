#pragma once

#include <Engine.Reflect/Inherit/InheritMeta.hpp>

#include "CollisionComponent.hpp"

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

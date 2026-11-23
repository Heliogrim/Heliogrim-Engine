#pragma once

#include <Engine.ACS/Component/SceneComponent.hpp>

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

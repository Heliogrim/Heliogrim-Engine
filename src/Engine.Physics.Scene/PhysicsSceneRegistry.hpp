#pragma once

#include <Engine.Scene.Support/SceneSystemTypeMappedRegistry.hpp>

#include "PhysicsSceneSystemModel.hpp"

namespace hg::engine::physics {
	class PhysicsSceneRegistry :
		public scene::SceneSystemTypeMappedRegistry<PhysicsSceneSystemModel> {
	public:
		using this_type = scene::SceneSystemTypeMappedRegistry<PhysicsSceneSystemModel>;
		using this_type::this_type;
	};
}

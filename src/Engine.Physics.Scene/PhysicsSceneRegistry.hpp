#pragma once

#include "PhysicsSceneSystemModel.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Scene.Support;
#endif

namespace hg::engine::physics {
	class PhysicsSceneRegistry :
		public scene::SceneSystemTypeMappedRegistry<PhysicsSceneSystemModel> {
	public:
		using this_type = scene::SceneSystemTypeMappedRegistry<PhysicsSceneSystemModel>;
		using this_type::this_type;
	};
}

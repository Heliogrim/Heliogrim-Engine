#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::scene {
	enum class SceneUpdateFlagBits : u8 {
		ePrePhysics,
		ePostPhysics
	};

	typedef Flag<SceneUpdateFlagBits> SceneUpdateFlags;
}

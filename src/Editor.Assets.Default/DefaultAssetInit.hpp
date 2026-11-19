#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine {
	class Engine;
}

namespace hg::editor::boot {
	extern void initDefaultFontAssets(ref<const engine::Engine> engine_);

	extern void initDefaultGraphicsMaterialAssets(ref<const engine::Engine> engine_);

	extern void initDefaultImageAssets(ref<const engine::Engine> engine_);

	extern void initDefaultMeshAssets(ref<const engine::Engine> engine_);

	extern void initDefaultTextureAssets(ref<const engine::Engine> engine_);
}

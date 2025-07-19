#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg::engine {
	class Engine;
}

namespace hg::editor::boot {
	extern void initDefaultFontAssets(ref<const engine::Engine> engine_);

	extern void initDefaultGfxMaterialAssets(ref<const engine::Engine> engine_);

	extern void initDefaultImageAssets(ref<const engine::Engine> engine_);

	extern void initDefaultMeshAssets(ref<const engine::Engine> engine_);

	extern void initDefaultTextureAssets(ref<const engine::Engine> engine_);
}

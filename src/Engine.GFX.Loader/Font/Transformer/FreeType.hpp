#pragma once

#include "../FontTransformer.hpp"

namespace hg::engine::gfx {
	class Device;
}

namespace hg::engine::gfx::loader::transformer {
	extern void convertFreeType(
		const non_owning_rptr<const assets::Font> assets_,
		mref<storage::AccessBlobReadonly> src_,
		nmpt<reflow::Font> dst_,
		cref<sptr<Device>> device_,
		const FontLoadOptions options_
	);

	/* Workaround for lifecycle */

	extern void prepareFreeType();

	extern void initFaceFromAsset(cref<assets::Font> asset_);

	extern void cleanupFreeType();
}

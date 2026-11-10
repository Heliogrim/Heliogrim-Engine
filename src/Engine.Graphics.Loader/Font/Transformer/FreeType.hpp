#pragma once

#include "../FontTransformer.hpp"

namespace hg::engine::gfx {
	class Device;
}

namespace hg::engine::gfx::loader::transformer {
	extern void convertFreeType(
		const non_owning_rptr<const assets::FontAsset> assets_,
		mref<std::pair<ref<storage::StorageSystem>, Arci<storage::IStorage>>> src_,
		nmpt<reflow::Font> dst_,
		cref<sptr<Device>> device_,
		const FontLoadOptions options_
	);

	/* Workaround for lifecycle */

	extern void prepareFreeType();

	extern void initFaceFromAsset(cref<assets::FontAsset> asset_);

	extern void cleanupFreeType();
}

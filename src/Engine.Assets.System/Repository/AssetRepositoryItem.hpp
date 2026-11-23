#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets.Type;
import Engine.Common;
#endif

namespace hg::engine::assets::system {
	struct AssetRepositoryItem {
		Arci<Asset> asset;
	};
}

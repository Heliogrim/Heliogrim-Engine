#pragma once

#include <Engine.Assets.Type/Asset.hpp>
#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::assets::system {
	struct AssetRepositoryItem {
		Arci<Asset> asset;
	};
}

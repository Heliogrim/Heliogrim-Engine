#pragma once

#include "__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets.Type;
#endif

namespace hg::engine::assets::system {
	struct AssetDescriptor {
		Arci<Asset> asset;
	};
}

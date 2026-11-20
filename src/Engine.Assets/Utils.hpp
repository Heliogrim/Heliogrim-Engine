#pragma once

#include <Engine.Common/Sal.hpp>

#include "AssetName.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::assets {
	class Asset;
}

namespace hg::engine::assets {
	[[nodiscard]] Arci<Asset> rename(_In_ mref<Arci<Asset>> asset_, _In_ mref<AssetName> newName_);
}

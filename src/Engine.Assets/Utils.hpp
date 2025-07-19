#pragma once

#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Common/Memory/UniquePointer.hpp>

#include "AssetName.hpp"

namespace hg::engine::assets {
	class Asset;
}

namespace hg::engine::assets {
	[[nodiscard]] Arci<Asset> rename(_In_ mref<Arci<Asset>> asset_, _In_ mref<AssetName> newName_);
}

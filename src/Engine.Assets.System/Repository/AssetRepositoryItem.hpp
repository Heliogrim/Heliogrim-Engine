#pragma once

#include <Engine.Assets/Types/Asset.hpp>
#include <Engine.Common/Managed/Rc.hpp>

namespace hg::engine::assets::system {
	struct AssetRepositoryItem {
		Arci<Asset> asset;
	};
}

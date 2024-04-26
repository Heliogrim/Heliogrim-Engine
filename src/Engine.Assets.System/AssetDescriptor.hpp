#pragma once

#include <Engine.Assets/Types/Asset.hpp>

#include "__fwd.hpp"

namespace hg::engine::assets::system {
	struct AssetDescriptor {
		Arci<Asset> asset;
	};
}

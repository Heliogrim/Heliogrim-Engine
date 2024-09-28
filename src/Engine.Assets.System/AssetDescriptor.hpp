#pragma once

#include <Engine.Assets.Type/Asset.hpp>

#include "__fwd.hpp"

namespace hg::engine::assets::system {
	struct AssetDescriptor {
		Arci<Asset> asset;
	};
}

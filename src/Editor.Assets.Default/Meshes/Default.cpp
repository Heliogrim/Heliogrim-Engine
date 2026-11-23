#include "Default.hpp"

import Engine.Assets;
import Engine.Assets.System;

using namespace hg;

Opt<Arci<engine::assets::Asset>> editor::assets::meshes::resolveRuntimeAsset(
	ref<engine::Engine> engine_,
	cref<AssetGuid> guid_
) {
	return engine_.getAssets()->getRegistry()->findAssetByGuid(guid_);
}

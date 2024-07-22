#include "Default.hpp"

#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>

using namespace hg;

nmpt<engine::assets::Asset> editor::assets::font::resolveRuntimeAsset(
	ref<engine::Engine> engine_,
	cref<asset_guid> guid_
) {
	return engine_.getAssets()->getRegistry()->findAssetByGuid(guid_);
}

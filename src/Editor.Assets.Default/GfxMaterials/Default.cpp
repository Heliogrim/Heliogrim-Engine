#include "Default.hpp"

#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>

using namespace hg;

Opt<Arci<engine::assets::Asset>> editor::assets::materials::resolveRuntimeAsset(
	ref<engine::Engine> engine_,
	cref<AssetGuid> guid_
) {
	return engine_.getAssets()->getRegistry()->findAssetByGuid(guid_);
}

#include "UniverseAssetHandles.hpp"

using namespace hg::engine::assets;
using namespace hg;

template <>
template <>
AssetTypeId TypedAssetHandle<LevelAsset>::getStaticTypeId<LevelAsset>() noexcept {
	return LevelAsset::typeId;
}

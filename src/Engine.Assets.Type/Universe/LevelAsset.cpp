#include "LevelAsset.hpp"

using namespace hg::engine::assets;
using namespace hg;

LevelAsset::LevelAsset(mref<AssetGuid> guid_) :
	InheritMeta(std::move(guid_), typeId) {}

cref<AssetReferenceUrl> LevelAsset::getLevelData() const noexcept {
	return _levelData;
}

void LevelAsset::setLevelData(mref<AssetReferenceUrl> url_) {
	_levelData = std::move(url_);
}

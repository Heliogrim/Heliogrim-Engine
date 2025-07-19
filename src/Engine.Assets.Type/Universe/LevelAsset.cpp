#include "LevelAsset.hpp"

using namespace hg::engine::assets;
using namespace hg;

LevelAsset::LevelAsset(
	mref<AssetGuid> guid_,
	mref<StringView> name_,
	mref<AssetReferenceUrl> storageUrl_,
	mref<AssetUrl> vfsUrl_
) :
	InheritMeta(
		std::move(guid_),
		typeId,
		::hg::move(name_),
		::hg::move(storageUrl_),
		::hg::move(vfsUrl_)
	) {}

cref<AssetReferenceUrl> LevelAsset::getLevelData() const noexcept {
	return _levelData;
}

void LevelAsset::setLevelData(mref<AssetReferenceUrl> url_) {
	_levelData = std::move(url_);
}

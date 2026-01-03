#include "LevelAsset.hpp"

#include <Engine.Level/Level.hpp>

using namespace hg::engine::assets;
using namespace hg;

LevelAsset::LevelAsset() noexcept :
	InheritMeta(clone(invalid_asset_guid), LevelAsset::typeId, StringView {}, AssetReferenceUrl {}, AssetUrl {}) {}

LevelAsset::LevelAsset(
	ref<const core::Level> level_,
	mref<StringView> name_,
	mref<AssetReferenceUrl> storageUrl_,
	mref<AssetUrl> vfsUrl_
) :
	InheritMeta(
		clone(level_.assetGuid()),
		LevelAsset::typeId,
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

#include "Utils.hpp"

#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Assets.Type/Asset.hpp>

using namespace hg::engine::assets;
using namespace hg;

Arci<Asset> engine::assets::rename(mref<Arci<Asset>> asset_, mref<AssetName> newName_) {
	::hg::assertd(asset_.ref_unique());

	asset_->setAssetName(clone(newName_));

	auto next_vfs_url = asset_->getAssetVfsUrl();
	next_vfs_url.setAssetName(::hg::move(newName_));
	asset_->setAssetVfsUrl(::hg::move(next_vfs_url));

	return ::hg::move(asset_);
}

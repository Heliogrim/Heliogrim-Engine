#include "Asset.hpp"

using namespace hg::engine::assets;
using namespace hg;

Asset::Asset(
	mref<AssetGuid> guid_,
	ref<const AssetTypeId> type_,
	mref<StringView> name_,
	mref<AssetReferenceUrl> storageUrl_,
	mref<AssetUrl> vfsUrl_
) :
	_guid(::hg::move(guid_)),
	_type(type_),
	_assetName(::hg::move(name_)),
	_assetStorageUrl(::hg::move(storageUrl_)),
	_assetVfsUrl(::hg::move(vfsUrl_)) {}

AssetGuid Asset::getAssetGuid() const noexcept {
	return _guid;
}

AssetTypeId Asset::getTypeId() const noexcept {
	return _type;
}

ref<const AssetName> Asset::getAssetName() const noexcept {
	return _assetName;
}

void Asset::setAssetName(mref<AssetName> assetName_) {
	_assetName = assetName_;
}

ref<const AssetReferenceUrl> Asset::getAssetStorageUrl() const noexcept {
	return _assetStorageUrl;
}

ref<const AssetUrl> Asset::getAssetVfsUrl() const noexcept {
	return _assetVfsUrl;
}

void Asset::setAssetStorageUrl(mref<AssetReferenceUrl> assetStorageUrl_) {
	_assetStorageUrl = ::hg::move(assetStorageUrl_);
}

void Asset::setAssetVfsUrl(mref<AssetUrl> assetVfsUrl_) {
	_assetVfsUrl = ::hg::move(assetVfsUrl_);
}

#include "Asset.hpp"

using namespace hg::engine::assets;
using namespace hg;

Asset::Asset(mref<AssetGuid> guid_, mref<AssetTypeId> type_) :
	_guid(std::move(guid_)),
	_type(std::move(type_)) {}

Asset::Asset(mref<AssetGuid> guid_, cref<AssetTypeId> type_) :
	_guid(std::move(guid_)),
	_type(type_) {}

AssetGuid Asset::get_guid() const noexcept {
	return _guid;
}

AssetTypeId Asset::getTypeId() const noexcept {
	return _type;
}

string_view Asset::getAssetName() const noexcept {
	return _assetName;
}

void Asset::setAssetName(string_view assetName_) {
	_assetName = assetName_;
}

string_view Asset::getVirtualUrl() const noexcept {
	return _virtualUrl;
}

void Asset::setVirtualUrl(string_view virtualUrl_) {
	_virtualUrl = virtualUrl_;
}

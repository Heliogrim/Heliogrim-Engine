#include "Asset.hpp"

using namespace hg::engine::assets;
using namespace hg;

Asset::Asset(cref<asset_guid> guid_, cref<asset_type_id> type_) :
    HeliogrimObject(),
    _guid(guid_),
    _type(type_) {}

Asset::~Asset() noexcept = default;

asset_guid Asset::get_guid() const noexcept {
    return _guid;
}

asset_type_id Asset::getTypeId() const noexcept {
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

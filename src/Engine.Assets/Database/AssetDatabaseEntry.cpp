#include "AssetDatabaseEntry.hpp"

using namespace ember::engine::assets;
using namespace ember;

AssetDatabaseEntry::AssetDatabaseEntry() noexcept {}

AssetDatabaseEntry::AssetDatabaseEntry(mref<value_type> other_) noexcept :
    _guid(_STD move(other_._guid)),
    _asset(_STD exchange(other_._asset, nullptr)) {}

AssetDatabaseEntry::AssetDatabaseEntry(cref<asset_guid> guid_, const ptr<Asset> asset_) noexcept :
    _guid(guid_),
    _asset(asset_) {}

AssetDatabaseEntry::~AssetDatabaseEntry() noexcept = default;

AssetDatabaseEntry::reference_type AssetDatabaseEntry::operator=(mref<value_type> other_) noexcept {

    if (this != _STD addressof(other_)) {
        _guid = _STD move(other_._guid);
        _asset = _STD exchange(other_._asset, nullptr);
    }

    return *this;
}

cref<asset_guid> AssetDatabaseEntry::guid() const noexcept {
    return _guid;
}

const ptr<Asset> AssetDatabaseEntry::asset() const noexcept {
    return _asset;
}

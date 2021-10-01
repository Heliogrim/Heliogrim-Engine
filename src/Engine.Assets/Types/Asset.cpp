#include "Asset.hpp"

using namespace ember;

Asset::Asset(cref<asset_guid> guid_) :
    _guid(guid_) {}

Asset::~Asset() noexcept = default;

asset_guid Asset::get_guid() const noexcept {
    return _guid;
}

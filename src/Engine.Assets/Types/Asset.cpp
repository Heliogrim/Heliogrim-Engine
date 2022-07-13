#include "Asset.hpp"

using namespace ember::engine::assets;
using namespace ember;

Asset::Asset(cref<asset_guid> guid_, cref<asset_type_id> type_) :
    EmberObject(),
    _guid(guid_),
    _type(type_) {}

Asset::~Asset() noexcept = default;

asset_guid Asset::get_guid() const noexcept {
    return _guid;
}

asset_type_id Asset::getTypeId() const noexcept {
    return _type;
}

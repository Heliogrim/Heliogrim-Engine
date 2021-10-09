#include "Asset.hpp"

using namespace ember;

Asset::Asset(cref<asset_guid> guid_, cref<asset_type_id> typeId_, ptr<void> internal_) noexcept :
    _guid(guid_),
    _typeId(typeId_),
    _internal(internal_) {}

Asset::~Asset() noexcept = default;

cref<asset_guid> Asset::guid() const noexcept {
    return _guid;
}

asset_guid Asset::guid() noexcept {
    return _guid;
}

Asset::operator const asset_guid&() const noexcept {
    return _guid;
}

Asset::operator asset_guid() noexcept {
    return _guid;
}

cref<asset_type_id> Asset::typeId() const noexcept {
    return _typeId;
}

asset_type_id Asset::typeId() noexcept {
    return _typeId;
}

Asset::operator const asset_type_id&() const noexcept {
    return _typeId;
}

Asset::operator asset_type_id() noexcept {
    return _typeId;
}

bool Asset::isEqualType(const ptr<const Asset> other_) const noexcept {
    return _typeId == other_->_typeId;
}

bool Asset::isValidType() const noexcept {
    return _typeId.data != NULL;
}

bool Asset::operator==(cref<asset_guid> guid_) const noexcept {
    return _guid == guid_;
}

bool Asset::operator!=(cref<asset_guid> guid_) const noexcept {
    return _guid != guid_;
}

bool Asset::operator==(cref<asset_type_id> typeId_) const noexcept {
    return _typeId == typeId_;
}

bool Asset::operator!=(cref<asset_type_id> typeId_) const noexcept {
    return _typeId != typeId_;
}

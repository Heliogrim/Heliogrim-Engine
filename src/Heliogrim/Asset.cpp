#include "Asset.hpp"

#include <Engine.Assets/Types/Asset.hpp>
#include <Engine.Common/Memory/Address.hpp>

using namespace hg;

Asset::Asset(mref<asset_guid> guid_, cref<asset_type_id> typeId_, ref<::hg::engine::assets::Asset> internal_) noexcept :
	_guid(std::move(guid_)),
	_typeId(typeId_),
	_internal(std::addressof(internal_)) {}

Asset::Asset(cref<asset_guid> invalidGuid_, cref<asset_type_id> invalidTypeId_, std::nullptr_t) noexcept :
	_guid(invalidGuid_),
	_typeId(invalidTypeId_),
	_internal(nullptr) {}

Asset::Asset(mref<this_type> other_) noexcept :
	_guid(std::move(other_._guid)),
	_typeId(other_._typeId),
	_internal(other_._internal) {}

Asset::Asset(cref<this_type> other_) :
	_guid(other_._guid),
	_typeId(other_._typeId),
	_internal(other_._internal) {}

Asset::~Asset() noexcept = default;

ref<Asset::this_type> Asset::operator=(mref<this_type> other_) noexcept {
	if (std::addressof(other_) != this) {
		_guid = std::move(other_._guid);
		_typeId = std::move(other_._typeId);
		_internal = std::move(other_._internal);
	}
	return *this;
}

ref<Asset::this_type> Asset::operator=(cref<this_type> other_) {
	if (std::addressof(other_) != this) {
		_guid = other_._guid;
		_typeId = other_._typeId;
		_internal = other_._internal;
	}
	return *this;
}

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

string_view Asset::getAssetName() const noexcept {
	if (_internal) {
		return _internal->getAssetName();
	}

	return {};
}

void Asset::setAssetName(string_view assetName_) {
	if (_internal) {
		_internal->setAssetName(assetName_);
	}
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

decltype(Asset::_internal) Asset::internal() const noexcept {
	return _internal;
}

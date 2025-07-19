#include "AssetHandle.hpp"

#include <Engine.Assets.Type/Asset.hpp>
#include <Engine.Common/Move.hpp>

using namespace hg;

AssetHandle::AssetHandle() noexcept = default;

AssetHandle::AssetHandle(mref<AssetGuid> guid_, ref<const AssetTypeId> type_, mref<Opt<ref<engine::assets::Asset>>> ref_) noexcept :
	_guid(::hg::move(guid_)),
	_type(type_),
	_ref(::hg::move(ref_)) {}

AssetHandle::AssetHandle(ref<const this_type> other_) noexcept = default;

AssetHandle::AssetHandle(mref<this_type> other_) noexcept = default;

ref<AssetHandle::this_type> AssetHandle::operator=(mref<this_type> other_) noexcept = default;

ref<AssetHandle::this_type> AssetHandle::operator=(ref<const this_type> other_) noexcept = default;

ref<const AssetGuid> AssetHandle::getAssetGuid() const noexcept {
	return _guid;
}

AssetHandle::operator const AssetGuid&() const noexcept {
	return _guid;
}

ref<const AssetTypeId> AssetHandle::getAssetTypeId() const noexcept {
	return _type;
}

AssetHandle::operator const AssetTypeId&() const noexcept {
	return _type;
}

bool AssetHandle::isValid() const noexcept {
	if (_ref == None) {
		return false;
	}

	return _ref->getAssetGuid() == _guid && _ref->getTypeId() == _type;
}

bool AssetHandle::operator==(ref<const AssetGuid> guid_) const noexcept {
	return _guid == guid_;
}

bool AssetHandle::operator==(ref<const AssetTypeId> type_) const noexcept {
	return _type == type_;
}

bool AssetHandle::operator==(ref<const AssetHandle> handle_) const noexcept {
	if (this == std::addressof(handle_)) {
		return true;
	}

	if ((_ref == None) != (handle_._ref == None)) {
		return false;
	}

	if (_ref == None && handle_._ref == None) {
		return _guid == handle_._guid && _type == handle_._type;
	}

	return std::addressof(*_ref) == std::addressof(*handle_._ref);
}

bool AssetHandle::operator!=(ref<const AssetGuid> guid_) const noexcept {
	return _guid != guid_;
}

bool AssetHandle::operator!=(ref<const AssetTypeId> type_) const noexcept {
	return _type != type_;
}

bool AssetHandle::operator!=(ref<const AssetHandle> handle_) const noexcept {
	if (this == std::addressof(handle_)) {
		return false;
	}

	if ((_ref == None) != (handle_._ref == None)) {
		return true;
	}

	if (_ref == None && handle_._ref == None) {
		return _guid != handle_._guid || _type != handle_._type;
	}

	return std::addressof(*_ref) != std::addressof(*handle_._ref);
}

std::strong_ordering AssetHandle::operator<=>(ref<const AssetHandle> other_) const noexcept {
	return _guid <=> other_._guid;
}

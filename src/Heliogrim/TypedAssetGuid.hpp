#pragma once

#include <concepts>
#include <utility>
#include <Engine.Assets/AssetGuid.hpp>

namespace hg::engine::assets {
	class Asset;
}

namespace hg {
	template <std::derived_from<::hg::engine::assets::Asset> AssetType_, bool Strict_ = true>
	class TypedAssetGuid {
	public:
		using this_type = TypedAssetGuid<AssetType_, Strict_>;
		using asset_type = AssetType_;

		constexpr static auto is_strict = Strict_;

	public:
		constexpr TypedAssetGuid() noexcept:
			_guid(invalid_asset_guid) {}

		template <bool FromStrict_ = Strict_> requires (FromStrict_ || FromStrict_ == Strict_)
		constexpr TypedAssetGuid(cref<TypedAssetGuid<asset_type, FromStrict_>> other_) noexcept :
			_guid(other_._guid) {}

		template <bool FromStrict_ = Strict_> requires (FromStrict_ || FromStrict_ == Strict_)
		constexpr TypedAssetGuid(mref<TypedAssetGuid<asset_type, FromStrict_>> other_) noexcept :
			_guid(std::exchange(other_._guid, invalid_asset_guid)) {}

		constexpr TypedAssetGuid(mref<asset_guid> assetGuid_) noexcept requires (not is_strict) :
			_guid(std::move(assetGuid_)) {}

		template <class FromAssetType_ = AssetType_> requires std::derived_from<FromAssetType_, AssetType_>
		constexpr TypedAssetGuid(ref<FromAssetType_> asset_) noexcept :
			_guid(asset_.get_guid()) {}

		template <class FromWeakAssetType_ = AssetType_> requires
			(not std::is_same_v<FromWeakAssetType_, AssetType_>) &&
			std::derived_from<AssetType_, FromWeakAssetType_> &&
			(not is_strict)
		constexpr TypedAssetGuid(ref<FromWeakAssetType_> asset_) noexcept :
			_guid(asset_->get_guid()) {}

		constexpr TypedAssetGuid(const this_type&) noexcept = default;

		constexpr TypedAssetGuid(this_type&&) noexcept = default;

		constexpr ~TypedAssetGuid() noexcept = default;

	public:
		template <class FromAssetType_ = AssetType_, bool FromStrict_ = Strict_> requires
			std::derived_from<FromAssetType_, AssetType_> &&
			(FromStrict_ || FromStrict_ == Strict_)
		constexpr ref<this_type> operator=(cref<TypedAssetGuid<FromAssetType_, FromStrict_>> other_) noexcept {
			if (std::addressof(other_) != this) {
				_guid = other_._guid;
			}
			return *this;
		}

		constexpr ref<this_type> operator=(cref<this_type> other_) noexcept = default;

		template <class FromAssetType_ = AssetType_, bool FromStrict_ = Strict_> requires
			std::derived_from<FromAssetType_, AssetType_> &&
			(FromStrict_ || FromStrict_ == Strict_)
		constexpr ref<this_type> operator=(mref<TypedAssetGuid<FromAssetType_, FromStrict_>> other_) noexcept {
			if (std::addressof(other_) != this) {
				_guid = std::exchange(other_._guid, invalid_asset_guid);
			}
			return *this;
		}

		constexpr ref<this_type> operator=(mref<this_type>) noexcept = default;

	private:
		asset_guid _guid;

	public:
		[[nodiscard]] constexpr bool valid() const noexcept {
			return _guid != invalid_asset_guid;
		}

		[[nodiscard]] constexpr operator hg::asset_guid() const noexcept {
			return _guid;
		}
	};
}

#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Hash/Fnv-1a.hpp>

#include "AssetName.hpp"
#include "AssetPath.hpp"

namespace hg::engine::assets {
	class AssetUrl final {
	public:
		using this_type = AssetUrl;

	public:
		constexpr AssetUrl() noexcept = default;

		constexpr AssetUrl(
			auto&& assetPath_,
			auto&& assetName_
			// Error: (MSVC) ) noexcept(std::is_nothrow_constructible_v<AssetPath, decltype(assetPath_)&&> && std::is_nothrow_constructible_v<AssetName, decltype(assetName_)&&>) :
		) noexcept :
			_path(::hg::forward<decltype(assetPath_)>(assetPath_)),
			_name(::hg::forward<decltype(assetName_)>(assetName_)) {}

		constexpr AssetUrl(ref<const this_type> other_) = default;

		constexpr AssetUrl(mref<this_type> other_) noexcept = default;

		constexpr ~AssetUrl() = default;

	public:
		ref<this_type> operator=(ref<const this_type> other_) = default;

		ref<this_type> operator=(mref<this_type> other_) noexcept = default;

	private:
		AssetPath _path;
		AssetName _name;

	public:
		void setAssetName(auto&& assetName_) noexcept(std::is_nothrow_assignable_v<AssetName, decltype(assetName_)>) {
			_name = ::hg::forward<decltype(assetName_)>(assetName_);
		}

		[[nodiscard]] ref<const AssetName> getAssetName() const noexcept {
			return _name;
		}

		void setAssetPath(auto&& assetPath_) noexcept(std::is_nothrow_assignable_v<AssetPath, decltype(assetPath_)>) {
			_path = std::forward<decltype(assetPath_)>(assetPath_);
		}

		[[nodiscard]] ref<const AssetPath> getAssetPath() const noexcept {
			return _path;
		}

	public:
		[[nodiscard]] constexpr bool empty() const noexcept {
			return _path.empty() && _name.empty();
		}

	public:
		[[nodiscard]] constexpr bool operator==(ref<const this_type> other_) const noexcept {
			return (this == std::addressof(other_)) || (_path == other_._path && _name == other_._name);
		}

		[[nodiscard]] constexpr bool operator!=(ref<const this_type> other_) const noexcept {
			return (this != std::addressof(other_)) && (_path != other_._path || _name != other_._name);
		}

		[[nodiscard]] constexpr std::strong_ordering operator <=>(ref<const this_type> other_) const noexcept {
			const auto result = _path <=> other_._path;
			return result == std::strong_ordering::equivalent ? _name <=> other_._name : result;
		}
	};
}

/**/

namespace std {
	template <>
	struct hash<::hg::engine::assets::AssetUrl> {
		[[nodiscard]] constexpr size_t operator()(::hg::ref<const ::hg::engine::assets::AssetUrl> assetUrl_) const noexcept {

			auto baseHash = ::hg::hash::fnv1a(assetUrl_.getAssetName());
			const auto pathHash = ::hg::hash::fnv1a(assetUrl_.getAssetPath().asByteSpan());

			hg::hash::hash_combine(baseHash, pathHash);
			return baseHash;
		}
	};
}

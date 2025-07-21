#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Filesystem/Path.hpp>

namespace hg::engine::assets {
	class AssetPath final {
	public:
		using this_type = AssetPath;

	public:
		constexpr AssetPath() noexcept = default;

		// Error: (MSVC) explicit constexpr AssetPath(auto&&... args_) noexcept(std::is_nothrow_constructible_v<String, decltype(args_)&&...>) :
		explicit constexpr AssetPath(auto&&... args_) noexcept :
			_data(::hg::forward<decltype(args_)>(args_)...) {}

		explicit AssetPath(ref<const fs::Path> path_) noexcept :
			_data(path_.normalized()) {}

		explicit AssetPath(mref<fs::Path> path_) noexcept :
			_data(::hg::move(path_).normalized()) {}

		constexpr AssetPath(ref<const this_type> other_) = default;

		constexpr AssetPath(mref<this_type> other_) noexcept = default;

		constexpr ~AssetPath() noexcept = default;

	public:
		ref<this_type> operator=(ref<const this_type> other_) = default;

		ref<this_type> operator=(mref<this_type> other_) noexcept = default;

	public:
		[[nodiscard]] constexpr bool empty() const noexcept {
			return _data.empty();
		}

		[[nodiscard]] constexpr bool contains(ref<const this_type> other_) const noexcept {
			return _data.starts_with(other_._data);
		}

	public:
		[[nodiscard]] constexpr bool operator==(ref<const this_type> other_) const noexcept = default;

		[[nodiscard]] constexpr bool operator!=(ref<const this_type> other_) const noexcept = default;

		[[nodiscard]] constexpr std::strong_ordering operator<=>(ref<const this_type> other_) const noexcept {
			return _data <=> other_._data;
		}

	private:
		String _data;

	public:
		[[nodiscard]] constexpr std::span<const char> asByteSpan() const noexcept {
			return _data;
		}
	};
}

#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Meta/Support.hpp>
#include <Engine.Filesystem/Path.hpp>

namespace hg::engine::assets {
	class AssetPath final {
	public:
		using this_type = AssetPath;

	public:
		constexpr AssetPath() noexcept = default;

		explicit constexpr AssetPath(auto&& arg0_, auto&&... args_) noexcept
			requires (not std::is_same_v<::hg::meta::peeled_t<decltype(arg0_)>, fs::Path>) :
			_data { ::hg::forward<decltype(arg0_)>(arg0_), ::hg::forward<decltype(args_)>(args_)... } {}

		explicit AssetPath(ref<const fs::Path> path_) :
			_data(normalizeOrThrow(path_.normalized())) {}

		explicit AssetPath(mref<fs::Path> path_) noexcept :
			_data(normalizeOrThrow(::hg::move(path_).normalized())) {}

		constexpr AssetPath(ref<const this_type> other_) = default;

		constexpr AssetPath(mref<this_type> other_) noexcept = default;

		constexpr ~AssetPath() noexcept = default;

	public:
		ref<this_type> operator=(ref<const this_type> other_) = default;

		ref<this_type> operator=(mref<this_type> other_) noexcept = default;

	private:
		[[nodiscard]] constexpr static String normalizeOrThrow(mref<fs::Path> denormalized_) {
			const auto converted = String { ::hg::move(denormalized_) };
			if (converted.starts_with(".."sv) || converted.starts_with("/"sv)) {
				throw std::runtime_error("Failed too consume denormalize path for asset path normalization.");
			}

			return converted.starts_with("."sv) ? ::hg::move(converted).substr(1uLL) : ::hg::move(converted);
		}

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

#pragma once

#include <Engine.Filesystem/Path.hpp>

#include "IUrlComp.hpp"

namespace hg::engine::storage {
	/**
	* @details The file url type wil lock-in a normalized form of the filesystem path provided.
	*	Thus we will drop the support for arbitrary path manipulation actions with freely defined content
	*	to uphold the guarantee of a normalized representation.
	*/
	class FileUrl final :
		public IUrlComp<FileUrl> {
	public:
		constexpr FileUrl() noexcept :
			IUrlComp(),
			_scheme(FileScheme),
			_path() {}

		FileUrl(auto&& scheme_, mref<fs::Path> path_) noexcept :
			IUrlComp(),
			_scheme(::hg::forward<decltype(scheme_)>(scheme_)),
			_path(std::move(path_).normalized()) {}

		constexpr FileUrl(ref<const FileUrl> other_) noexcept = default;

		constexpr FileUrl(mref<FileUrl> other_) noexcept = default;

		constexpr ~FileUrl() noexcept = default;

	public:
		constexpr ref<FileUrl> operator=(ref<const FileUrl> other_) noexcept = default;

		constexpr ref<FileUrl> operator=(mref<FileUrl> other_) noexcept = default;

	private:
		UrlScheme _scheme;
		fs::Path _path;

	public:
		[[nodiscard]] constexpr UrlScheme scheme() const noexcept {
			return _scheme;
		}

		[[nodiscard]] constexpr ref<const fs::Path> path() const noexcept {
			return _path;
		}

		[[nodiscard]] String encode() const;

		[[nodiscard]] constexpr bool empty() const noexcept {
			// Note: We explicitly ignore the empty state of the `_scheme` value.
			return _path.empty();
		}
	};
}

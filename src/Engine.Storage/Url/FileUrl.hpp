#pragma once

#include <Engine.Filesystem/Path.hpp>

#include "IUrlComp.hpp"

namespace hg::engine::storage {
	class FileUrl final :
		public IUrlComp<FileUrl> {
	public:
		constexpr FileUrl() noexcept :
			IUrlComp(),
			_scheme(FileScheme),
			_path() {}

		constexpr FileUrl(mref<UrlScheme> scheme_, mref<fs::Path> path_) noexcept :
			IUrlComp(),
			_scheme(std::move(scheme_)),
			_path(std::move(path_)) {}

		constexpr FileUrl(cref<FileUrl> other_) noexcept = default;

		constexpr FileUrl(mref<FileUrl> other_) noexcept = default;

		constexpr ~FileUrl() noexcept = default;

	public:
		constexpr ref<FileUrl> operator=(cref<FileUrl> other_) noexcept = default;

		constexpr ref<FileUrl> operator=(mref<FileUrl> other_) noexcept = default;

	private:
		UrlScheme _scheme;
		fs::Path _path;

	public:
		[[nodiscard]] constexpr UrlScheme scheme() const noexcept {
			return _scheme;
		}

		[[nodiscard]] constexpr fs::Path path() const noexcept {
			return _path;
		}

		[[nodiscard]] String encode() const;
	};
}

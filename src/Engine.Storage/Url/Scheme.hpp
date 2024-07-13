#pragma once

#include <concepts>
#include <Engine.Common/String.hpp>

namespace hg::engine::storage {
	/**/

	using RegisteredUrlScheme = size_t;

	/**/

	class UrlScheme :
		public StringView {
	public:
		using this_type = UrlScheme;
		using underlying_type = StringView;

	public:
		using underlying_type::underlying_type;

		constexpr UrlScheme(mref<underlying_type> value_) noexcept :// NOLINT(*-explicit-constructor)
			underlying_type(value_) {}

		constexpr UrlScheme(cref<this_type>) noexcept = default;

		constexpr UrlScheme(mref<this_type>) noexcept = default;

		constexpr ref<UrlScheme> operator=(cref<this_type>) noexcept = default;

		constexpr ref<UrlScheme> operator=(mref<this_type>) noexcept = default;
	};

	constexpr static UrlScheme ArchiveScheme = UrlScheme { "archive" };
	constexpr static UrlScheme FileScheme = UrlScheme { "file" };
	constexpr static UrlScheme FileCacheScheme = UrlScheme { "file+cache" };
	constexpr static UrlScheme FileEditorScheme = UrlScheme { "file+editor" };
	constexpr static UrlScheme FileRuntimeScheme = UrlScheme { "file+runtime" };
	constexpr static UrlScheme FileProjectScheme = UrlScheme { "file+project" };
	constexpr static UrlScheme MemoryScheme = UrlScheme { "mem" };
	constexpr static UrlScheme PackageScheme = UrlScheme { "package" };
}

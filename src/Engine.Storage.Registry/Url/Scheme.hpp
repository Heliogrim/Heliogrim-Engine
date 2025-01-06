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

	constexpr inline auto ArchiveScheme = UrlScheme { "archive"sv };
	constexpr inline auto FileScheme = UrlScheme { "file"sv };
	constexpr inline auto FileCacheScheme = UrlScheme { "file+cache"sv };
	constexpr inline auto FileEditorScheme = UrlScheme { "file+editor"sv };
	constexpr inline auto FileRuntimeScheme = UrlScheme { "file+runtime"sv };
	constexpr inline auto FileProjectScheme = UrlScheme { "file+project"sv };
	constexpr inline auto MemoryScheme = UrlScheme { "mem"sv };
	constexpr inline auto PackageScheme = UrlScheme { "package"sv };
}

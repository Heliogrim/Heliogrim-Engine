#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Serialization/Access/__fwd.hpp>
#include <Engine.Filesystem/__fwd.hpp>
#include <Engine.Filesystem/Path.hpp>

namespace hg::engine::fs {
	class Url {
	public:
		ACCESS_LAYOUT
		ACCESS_STRUCTURE

	public:
		using this_type = Url;

	public:
		constexpr Url() noexcept = default;

		Url(cref<this_type> other_) = default;

		Url(mref<this_type> other_) noexcept = default;

		Url(StringView scheme_, mref<Path> path_);

		template <typename PathStringType_> requires std::is_constructible_v<fs::Path, PathStringType_>
		Url(string_view scheme_, PathStringType_&& pathString_) :
			Url(scheme_, fs::Path { std::forward<PathStringType_>(pathString_) }) {}

		~Url();

	public:
		ref<this_type> operator=(cref<this_type> other_) = default;

		ref<this_type> operator=(mref<this_type> other_) noexcept = default;

	private:
		string _scheme;
		Path _path;

	public:
		[[nodiscard]] bool hasScheme() const noexcept;

		[[nodiscard]] const string_view scheme() const noexcept;

		ref<this_type> setScheme(string_view scheme_);

		[[nodiscard]] bool hasPath() const noexcept;

		[[nodiscard]] cref<Path> path() const noexcept;

		ref<this_type> setPath(mref<Path> path_);

	public:
		[[nodiscard]] bool empty() const noexcept;

	public:
		[[nodiscard]] string encode() const noexcept;
	};
}

#pragma once

#include <string>
#include <string_view>

namespace hg {
	using cstr = const char*;
	using string = std::string;
	using string_view = std::string_view;

	using String = string;
	using StringView = string_view;

	using namespace ::std::literals::string_view_literals;

	[[nodiscard]] constexpr String make_string_from_repeated(const char ch_, const String::size_type count_) {
		return String { count_, ch_, String::allocator_type {} };
	}

	[[nodiscard]] constexpr String make_null_string(const String::size_type count_) {
		return make_string_from_repeated(0, count_);
	}
}

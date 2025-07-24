#pragma once

#include <concepts>
#include <type_traits>

#include "Compile/TypeName.hpp"

namespace hg::refl {
	template <typename Type_>
	struct TypeNameFn;

	/**/

	// Attention: We need to deal with void as as special case, as void is ALWAYS an incomplete type
	template <>
	struct TypeNameFn<void> {
		[[nodiscard]] consteval StringView operator()() const noexcept {
			return "void"sv;
		}
	};

	/**/

	template <typename Type_> requires (not std::is_same_v<Type_, void>)
	struct TypeNameFn<Type_> {
		[[nodiscard]] consteval StringView operator()() const noexcept {
			return ctname<Type_>();
		}
	};

	/**/

	template <typename Type_>
	constexpr inline TypeNameFn<Type_> TypeName {};
}

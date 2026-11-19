#pragma once

#include "Compile/TypeName.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

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

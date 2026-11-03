#pragma once

#include <type_traits>

namespace hg {
	/**
	 * Engine contract: whether a type may be moved by raw byte copy.
	 * True by default; a type opts out by specializing to `std::false_type`.
	 */
	template <typename Ty_>
	struct relocatable : std::true_type {};

	template <typename Ty_>
	inline constexpr bool relocatable_v = relocatable<Ty_>::value;
}

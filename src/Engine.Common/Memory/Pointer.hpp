#pragma once

namespace hg {
	template <typename Type_>
	using owner_ptr = Type_*;

	/* C++ Language Extensions */

	/**
	 * Helper wrapper which will implicitly apply the restricted attribute to the requested pointer type
	 *
	 * @note The `__restrict` specification is provided either from MSVC or the alias macro of `__macro.hpp`
	 */
	template <typename Type_>
	using __restricted_ptr = Type_* __restrict;
}

#pragma once

#include <memory>
#include "__macro.hpp"

#include "Memory/SharedPointer.hpp"
#include "Memory/UniquePointer.hpp"
#include "Memory/__fwd.hpp"

namespace hg {
	template <class Ty>
	using ref = Ty&;

	template <class Ty>
	using cref = const Ty&;

	template <class Ty>
	using mref = Ty&&;

	template <class Ty>
	using ptr = Ty*;

	template <class Ty>
	using pointer = Ty*;

	template <typename Type_>
	using non_owning_rptr = Type_*;

	template <class Ty>
	using sptr = std::shared_ptr<Ty>;

	template <class Ty>
	using wptr = std::weak_ptr<Ty>;

	template <class Ty>
	using uptr = std::unique_ptr<Ty>;

	template <class Ty>
	using managed = std::shared_ptr<Ty>;

	/* C++ Language Extensions */

	/**
	 * Helper wrapper which will implicitly apply the restricted attribute to the requested pointer type
	 *
	 * @note The `__restrict` specification is provided either from MSVC or the alias macro of `__macro.hpp`
	 */
	template <typename Type_>
	using __restricted_ptr = Type_* __restrict;
}

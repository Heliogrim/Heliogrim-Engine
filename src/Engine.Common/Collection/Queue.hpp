#pragma once

#include "../__macro.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

namespace hg {
	template <class Type_>
	using queue = std::queue<Type_>;

	template <class Type_>
	using Queue = std::queue<Type_>;
}

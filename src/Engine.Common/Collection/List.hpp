#pragma once

#include "../__macro.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

namespace hg {
	template <class Type_, class Allocator_ = std::allocator<Type_>>
	using list = std::list<Type_, Allocator_>;
}

#pragma once

#include "../__macro.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

namespace hg {
	template <class Type_, class Allocator_ = std::allocator<Type_>>
	using Deque = std::deque<Type_, Allocator_>;
}

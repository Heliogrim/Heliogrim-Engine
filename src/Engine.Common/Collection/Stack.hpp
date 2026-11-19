#pragma once

#include "../__macro.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

namespace hg {
	template <class Type_, class ContainerType_ = std::deque<Type_>>
	using Stack = std::stack<Type_, ContainerType_>;
}

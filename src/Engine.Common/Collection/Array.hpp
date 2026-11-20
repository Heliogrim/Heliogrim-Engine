#pragma once

#include "../__macro.hpp"
#include "../Types.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

namespace hg {
	template <class Ty, size_t Size>
	using Array = std::array<Ty, Size>;
}

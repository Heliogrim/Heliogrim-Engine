#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#else
// Hoisted into a global module fragment the import is unavailable; Types.hpp typedefs merge with the exported ones.
#include "Types.hpp"
#endif

namespace hg {
	struct Buffer {
		u64 size = 0uLL;
		u64 align = 0uLL;

		void* mem = nullptr;
	};
}

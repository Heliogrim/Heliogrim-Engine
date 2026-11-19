#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg {
	struct Buffer {
		u64 size = 0uLL;
		u64 align = 0uLL;

		void* mem = nullptr;
	};
}

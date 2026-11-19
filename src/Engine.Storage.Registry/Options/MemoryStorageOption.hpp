#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::storage {
	/**/

	enum class MemoryProperty {
		eUndefined = 0x0,
		eProtected = 0x7
	};

	using MemoryRequestSize = u64;
}

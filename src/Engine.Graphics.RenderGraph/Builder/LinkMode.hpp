#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render::graph {
	enum class LinkMode : u8 {
		eInclusive = 0x0,
		eExclusive = 0x1
	};
}

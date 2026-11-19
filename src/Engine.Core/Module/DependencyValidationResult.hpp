#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::core {
	enum class DependencyValidationResult : u8 {
		eSuccess = 0x0,
		//
		eFailedRequired = 0x1,
		eFailedOrder = 0x2,
		eFailedUnique = 0x3
	};
}

#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg {
	/**
	 * The basic data type for ticks to enforce type safety
	 *
	 * @author Julius
	 * @date 23.10.2021
	 */
	struct ticks {
		u64 unwrap;
	};
}

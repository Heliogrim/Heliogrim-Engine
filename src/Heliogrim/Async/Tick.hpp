#pragma once

#include <Engine.Common/Types.hpp>

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

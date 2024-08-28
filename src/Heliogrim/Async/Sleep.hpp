#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Types.hpp>

namespace hg {
	/**
	 * Yields the current execution context at least for given milliseconds
	 *
	 * @author Julius
	 * @date 23.10.2021
	 *
	 * @param  milliseconds_ The milliseconds suspended at least.
	 */
	extern void sleep(_In_ u64 milliseconds_);
}

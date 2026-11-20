#pragma once

#include "../Diagnostic.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel::lang::ir::spirv {
	/**
	 * Runs the spirv-opt performance recipe for Vulkan 1.3 over `words_` in place.
	 * On failure `words_` stays unoptimized and the result holds one `internal-spirv-optimize` warning naming `module_`.
	 */
	[[nodiscard]] Diagnostics optimize(ref<Vector<u32>> words_, const StringView module_);
}

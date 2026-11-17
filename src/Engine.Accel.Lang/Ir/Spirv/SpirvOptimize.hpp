#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "../Diagnostic.hpp"

namespace hg::engine::accel::lang::ir::spirv {
	/**
	 * Runs the spirv-opt performance recipe for Vulkan 1.3 over `words_` in place.
	 * On failure `words_` stays unoptimized and the result holds one `internal-spirv-optimize` warning naming `module_`.
	 */
	[[nodiscard]] Diagnostics optimize(ref<Vector<u32>> words_, const StringView module_);
}

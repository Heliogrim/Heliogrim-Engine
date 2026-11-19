#pragma once

#include <Engine.Common/__macro.hpp>

#include "../Diagnostic.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel::lang::ir::spirv {
	/**
	 * Validates against Vulkan 1.3; every validator message becomes one `internal-spirv-invalid` error naming `module_`.
	 */
	[[nodiscard]] Diagnostics validate(const std::span<const u32> words_, const StringView module_);
}

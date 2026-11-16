#pragma once

#include <span>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Types.hpp>

#include "../Diagnostic.hpp"

namespace hg::engine::accel::lang::ir::spirv {
	/**
	 * Validates against Vulkan 1.3; every validator message becomes one `internal-spirv-invalid` error naming `module_`.
	 */
	[[nodiscard]] Diagnostics validate(const std::span<const u32> words_, const StringView module_);
}

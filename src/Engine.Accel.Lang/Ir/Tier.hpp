#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::accel::lang::ir {
	/**
	 * Emission depth: build (bare module, spirv-opt performance recipe), dev (names, OpLine debug lines and validation), deep (emits as dev).
	 */
	enum class Tier : u8 {
		eBuild,
		eDev,
		eDeep
	};
}

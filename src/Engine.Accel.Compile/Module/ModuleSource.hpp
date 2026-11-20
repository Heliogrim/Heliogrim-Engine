#pragma once

#include "ModuleTargetStage.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel {
	struct ModuleSource {
		constexpr virtual ~ModuleSource() noexcept = default;

		ModuleTargetStage targetStage;
		Vector<u32> spirv;
	};
}

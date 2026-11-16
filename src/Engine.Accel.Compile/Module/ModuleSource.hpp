#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "ModuleTargetStage.hpp"

namespace hg::engine::accel {
	struct ModuleSource {
		constexpr virtual ~ModuleSource() noexcept = default;

		ModuleTargetStage targetStage;
		Vector<u32> spirv;
	};
}

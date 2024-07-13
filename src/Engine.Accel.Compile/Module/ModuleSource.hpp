#pragma once

#include <Engine.Accel.Lang/Language.hpp>

#include "ModuleTargetStage.hpp"

namespace hg::engine::accel {
	struct ModuleSource {
		constexpr virtual ~ModuleSource() noexcept = default;

		ModuleTargetStage targetStage;
		lang::Language code;
	};
}

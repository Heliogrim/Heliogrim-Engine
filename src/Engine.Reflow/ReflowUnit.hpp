#pragma once

#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::reflow {
	enum class ReflowUnitType {
		eAuto,
		eAbsolute,
		eRelative
	};

	struct ReflowUnit {
		ReflowUnitType type = ReflowUnitType::eAuto;
		float value = 0.F;

		[[nodiscard]] constexpr bool operator==(ref<const ReflowUnit>) const noexcept = default;
	};
}

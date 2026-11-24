#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

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

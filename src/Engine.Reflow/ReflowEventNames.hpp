#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	struct ReflowEventNames {
		constexpr static auto BlurEvent = "BlurEvent"sv;
		constexpr static auto FocusEvent = "FocusEvent"sv;

		constexpr static auto ClickEvent = "ClickEvent"sv;
		constexpr static auto MouseEnterEvent = "MouseEnterEvent"sv;

		constexpr static auto InputChangeEvent = "InputChangeEvent"sv;
	};
}

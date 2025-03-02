#pragma once

#include <Engine.Common/String.hpp>

namespace hg::engine::reflow {
	struct ReflowEventNames {
		constexpr static auto BlurEvent = "BlurEvent"sv;
		constexpr static auto FocusEvent = "FocusEvent"sv;

		constexpr static auto ClickEvent = "ClickEvent"sv;
	};
}

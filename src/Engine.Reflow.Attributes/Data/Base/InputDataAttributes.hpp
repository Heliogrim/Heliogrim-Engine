#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow::attr {
	struct InputData {
		inline constexpr static auto limits = "limits"sv;
		inline constexpr static auto precision = "precision"sv;
		inline constexpr static auto step = "step"sv;
		inline constexpr static auto value = "value"sv;
	};
}

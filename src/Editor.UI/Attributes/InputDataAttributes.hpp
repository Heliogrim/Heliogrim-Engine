#pragma once

#include <Engine.Common/StringLiterals.hpp>
#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow::attr {
	struct InputData {
		constexpr static inline auto limits = "limits"sv;
		constexpr static inline auto precision = "precision"sv;
		constexpr static inline auto step = "step"sv;
		constexpr static inline auto value = "value"sv;
	};
}
#pragma once

#include <Engine.Common/String.hpp>

namespace hg::engine::reflow::attr {
	struct InputData {
		constexpr static inline auto limits = "limits"sv;
		constexpr static inline auto precision = "precision"sv;
		constexpr static inline auto step = "step"sv;
		constexpr static inline auto value = "value"sv;
	};
}
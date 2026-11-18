#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::scheduler {
	/**
	 * Queue lanes, high to low; eInherit is a request to resolve the lane and never a queue index.
	 */
	enum class Lane : u8 {
		eFrame,
		eInteractive,
		eBackground,
		eInherit
	};

	[[nodiscard]] constexpr Lane higher(const Lane a_, const Lane b_) noexcept {
		return static_cast<u8>(a_) <= static_cast<u8>(b_) ? a_ : b_;
	}

	[[nodiscard]] constexpr Lane lower(const Lane a_, const Lane b_) noexcept {
		return static_cast<u8>(a_) >= static_cast<u8>(b_) ? a_ : b_;
	}
}

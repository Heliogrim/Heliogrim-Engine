#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::reflow {
	enum class ReflowHorizontalPlacement : u8 {
		eLeft = 0b00,
		eCenter = 0b01,
		eRight = 0b10
	};

	enum class ReflowVerticalPlacement : u8 {
		eTop = 0b00,
		eMiddle = 0b01,
		eBottom = 0b10
	};

	enum class ReflowPlacement : u8 {
		eTopLeft = static_cast<u8>(ReflowVerticalPlacement::eTop) | (static_cast<u8>(ReflowHorizontalPlacement::eLeft) << 2),
		eTopCenter = static_cast<u8>(ReflowVerticalPlacement::eTop) | (static_cast<u8>(ReflowHorizontalPlacement::eCenter) << 2),
		eTopRight = static_cast<u8>(ReflowVerticalPlacement::eTop) | (static_cast<u8>(ReflowHorizontalPlacement::eRight) << 2),
		//
		eMiddleLeft = static_cast<u8>(ReflowVerticalPlacement::eMiddle) | (static_cast<u8>(ReflowHorizontalPlacement::eLeft) << 2),
		eMiddleCenter = static_cast<u8>(ReflowVerticalPlacement::eMiddle) | (static_cast<u8>(ReflowHorizontalPlacement::eCenter) << 2),
		eMiddleRight = static_cast<u8>(ReflowVerticalPlacement::eMiddle) | (static_cast<u8>(ReflowHorizontalPlacement::eRight) << 2),
		//
		eBottomLeft = static_cast<u8>(ReflowVerticalPlacement::eBottom) | (static_cast<u8>(ReflowHorizontalPlacement::eLeft) << 2),
		eBottomCenter = static_cast<u8>(ReflowVerticalPlacement::eBottom) | (static_cast<u8>(ReflowHorizontalPlacement::eCenter) << 2),
		eBottomRight = static_cast<u8>(ReflowVerticalPlacement::eBottom) | (static_cast<u8>(ReflowHorizontalPlacement::eRight) << 2),
	};
}

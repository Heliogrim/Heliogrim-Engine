#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::reflow {
	enum class TextAlign : u8 {
		eTopLeft = 0b0001'0001,
		eMiddleLeft = 0b0010'0001,
		eBottomLeft = 0b0100'0001,
		eTopCenter = 0b0001'0010,
		eMiddleCenter = 0b0010'0010,
		eBottomCenter = 0b0100'0010,
		eTopRight = 0b0001'0100,
		eMiddleRight = 0b0010'0100,
		eBottomRight = 0b0100'0100
	};
}

#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	enum class InteractiveStateFlagBits : u8 {
		eFocus = 0b0000'0001,
		eHover = 0b0000'0010,
		eDisable = 0b0000'0100,
		ePress = 0b0000'1000,
		eDrag = 0b0001'0000,
		eSelected = 0b0010'0000,
		eInvalid = 0b0100'0000
	};

	using StyleStates = Flag<InteractiveStateFlagBits, u8>;
}

#include "ComponentStyle.hpp"

import Engine.Common;

using namespace hg::engine::reflow::theming;
using namespace hg::engine::reflow;
using namespace hg;

u32 hg::engine::reflow::theming::stateLayerOrder(const StyleStates mask_) noexcept {

	constexpr auto precedence = std::array {
		InteractiveStateFlagBits::eSelected,
		InteractiveStateFlagBits::eHover,
		InteractiveStateFlagBits::eFocus,
		InteractiveStateFlagBits::ePress,
		InteractiveStateFlagBits::eDrag,
		InteractiveStateFlagBits::eInvalid,
		InteractiveStateFlagBits::eDisable
	};

	u32 rank = 0uL;
	for (u32 index = 0uL; index < precedence.size(); ++index) {
		if (mask_.unwrap & static_cast<u8>(precedence[index])) {
			rank = index + 1uL;
		}
	}

	return rank * 8uL + static_cast<u32>(std::popcount(mask_.unwrap));
}

bool hg::engine::reflow::theming::stateLayerMatches(const StyleStates mask_, const StyleStates states_) noexcept {
	return (states_.unwrap & mask_.unwrap) == mask_.unwrap;
}

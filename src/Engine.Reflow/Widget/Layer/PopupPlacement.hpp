#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	enum class PopupSide : u8 {
		eBelow,
		eAbove,
		eRight,
		eLeft
	};

	enum class PopupAlign : u8 {
		eStart,
		eCenter,
		eEnd
	};

	struct PopupAnchor {
		math::vec2 offset;
		math::vec2 size;
		PopupSide side;
		PopupAlign align;
	};

	/**
	 * Places a popup next to its anchor; flips, shifts and clamps so the top-left stays inside the client area.
	 */
	[[nodiscard]] math::vec2 placePopup(
		ref<const PopupAnchor> anchor_,
		const math::vec2 popupSize_,
		const math::vec2 clientSize_
	) noexcept;
}

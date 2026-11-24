#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	enum class LayerBand : u8 {
		eContent = 0,
		eDockOverlay,
		ePopup,
		eDialog,
		eNotification,
		eDragPreview,
		eTooltip
	};

	[[nodiscard]] bool isHitTestable(const LayerBand band_) noexcept;
}

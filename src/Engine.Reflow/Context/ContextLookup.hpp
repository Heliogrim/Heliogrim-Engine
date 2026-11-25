#pragma once

#include "../Widget/Widget.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	/**
	 * Walks the parents of the widget, not the widget itself, to the nearest provider of the context type.
	 *
	 * @details Popup content ends at its host, so it resolves up to its host and window, as themes do.
	 */
	[[nodiscard]] nmpt<Widget> findContextProvider(ref<const Widget> from_, const type_id contextType_) noexcept;
}

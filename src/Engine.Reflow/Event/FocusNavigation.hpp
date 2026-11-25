#pragma once

#include "../FocusPath.hpp"
#include "../Widget/__fwd.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	class FocusScope;

	struct FocusSnapshot {
		FocusPath path;
	};

	/**
	 * Closest enclosing scope of the widget (the widget itself included); null means the implicit window scope.
	 */
	[[nodiscard]] sptr<FocusScope> nearestScope(ref<const sptr<Widget>> widget_);

	/**
	 * Next visible focusable widget after `current_` in depth-first child order below `scopeRoot_`, wrapping.
	 */
	[[nodiscard]] sptr<Widget> nextFocusable(ref<const sptr<Widget>> scopeRoot_, ref<const sptr<Widget>> current_, const bool backwards_);
}

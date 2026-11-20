#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	struct LayoutContext {
		math::vec2 localOffset;
		math::vec2 localSize;
		float localScale;
	};
}

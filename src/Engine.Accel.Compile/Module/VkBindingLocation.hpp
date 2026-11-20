#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::accel {
	struct VkBindingLocation final {
		s32 set;
		s32 location;
	};
}

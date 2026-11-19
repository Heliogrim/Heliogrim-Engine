#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::graphics::types {
	struct QueueFamilyIndices {
		int32_t computeFamily = -1;
		int32_t transferFamily = -1;
		int32_t graphicsFamily = -1;

		[[nodiscard]] constexpr bool isComplete() const {
			return graphicsFamily >= 0 && computeFamily >= 0 && transferFamily >= 0;
		}
	};
}
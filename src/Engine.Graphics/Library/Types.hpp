#pragma once

#include <Engine.Common/Types.hpp>

namespace hg::engine::gfx::types {
	struct QueueFamilyIndices {
		int32_t computeFamily = -1;
		int32_t transferFamily = -1;
		int32_t graphicsFamily = -1;

		[[nodiscard]] constexpr bool isComplete() const {
			return graphicsFamily >= 0 && computeFamily >= 0 && transferFamily >= 0;
		}
	};
}
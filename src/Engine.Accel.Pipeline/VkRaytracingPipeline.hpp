#pragma once

#include <Engine.Graphics/API/__vkFwd.hpp>

#include "RaytracingPipeline.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Graphics;
#endif

namespace hg::engine::accel {
	class VkRaytracingPipeline final :
		public RaytracingPipeline {
	public:
		using this_type = VkRaytracingPipeline;

	public:
		~VkRaytracingPipeline() override = default;
	};
}

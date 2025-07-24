#pragma once

#include <Engine.GFX/API/__vkFwd.hpp>

#include "RaytracingPipeline.hpp"

namespace hg::engine::accel {
	class VkRaytracingPipeline final :
		public RaytracingPipeline {
	public:
		using this_type = VkRaytracingPipeline;

	public:
		~VkRaytracingPipeline() override = default;
	};
}

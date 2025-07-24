#pragma once

#include <Engine.GFX/API/__vkFwd.hpp>

#include "MeshPipeline.hpp"

namespace hg::engine::accel {
	class VkMeshPipeline final :
		public MeshPipeline {
	public:
		using this_type = VkMeshPipeline;

	public:
		~VkMeshPipeline() override = default;
	};
}

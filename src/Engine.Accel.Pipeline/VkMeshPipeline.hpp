#pragma once

#include <Engine.Graphics/API/__vkFwd.hpp>

#include "MeshPipeline.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Graphics;
#endif

namespace hg::engine::accel {
	class VkMeshPipeline final :
		public MeshPipeline {
	public:
		using this_type = VkMeshPipeline;

	public:
		~VkMeshPipeline() override = default;
	};
}

#pragma once

#include <Engine.Accel.Command/CommandBufferStructs.hpp>
#include <Engine.Accel.Pass/Framebuffer/Framebuffer.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render::cmd {
	struct BeginAccelerationPassStruct {
		nmpt<const accel::AccelerationPass> pass;
		nmpt<const graphics::Framebuffer> framebuffer;

		/**/

		// TODO: Rework
		Vector<vk::ClearValue> clearValues {};
	};

	struct BeginSubPassStruct {
		nmpt<const graphics::Framebuffer> framebuffer = nullptr;
	};
}

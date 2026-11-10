#pragma once

#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Graphics/Framebuffer/Framebuffer.hpp>

#include <Engine.Accel.Command/CommandBufferStructs.hpp>

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

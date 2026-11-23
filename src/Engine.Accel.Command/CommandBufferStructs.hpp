#pragma once
#include <Engine.Accel.Pass/GraphicsPass.hpp>
#include <Engine.Accel.Pass/Framebuffer/Framebuffer.hpp>
#include <Engine.Graphics/vkinc.hpp>

namespace hg::engine::accel {
	struct BindRenderPassStruct final {
		cref<GraphicsPass> graphicsPass;
		cref<graphics::Framebuffer> framebuffer;
		bool subrecording : 1 = false;

		/**/

		// TODO: Rework
		cref<Vector<vk::ClearValue>> clearValues {};
	};
}

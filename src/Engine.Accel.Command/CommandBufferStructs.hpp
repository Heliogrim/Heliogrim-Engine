#pragma once
#include <Engine.Accel.Pass/GraphicsPass.hpp>
#include <Engine.Graphics/vkinc.hpp>
#include <Engine.Graphics/Framebuffer/Framebuffer.hpp>

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

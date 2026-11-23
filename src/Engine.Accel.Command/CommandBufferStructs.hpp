#pragma once
#include <Engine.Graphics/vkinc.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Accel.Pass;
#endif

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

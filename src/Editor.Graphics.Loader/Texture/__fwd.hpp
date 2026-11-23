#pragma once

#include <Engine.Graphics.Loader/Texture/__fwd.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets.Type;
import Engine.Resource;
#endif

namespace hg::editor::graphics {}

namespace hg::editor::graphics::loader {
	class EditorTextureLoader;
	class EditorTextureCache;
	class EditorTextureFeedback;
	class EditorTextureTransformer;
}

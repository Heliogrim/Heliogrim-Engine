#pragma once

#include <Engine.Graphics.Loader/Texture/__fwd.hpp>
#include <Engine.Resource/Loader/__fwd.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets.Type;
#endif

namespace hg::editor::graphics {}

namespace hg::editor::graphics::loader {
	class EditorTextureLoader;
	class EditorTextureCache;
	class EditorTextureFeedback;
	class EditorTextureTransformer;
}

#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets.Type;
import Engine.Graphics.Loader;
import Engine.Resource;
#endif

namespace hg::editor::graphics {}

namespace hg::editor::graphics::loader {
	class EditorTextureLoader;
	class EditorTextureCache;
	class EditorTextureFeedback;
	class EditorTextureTransformer;
}

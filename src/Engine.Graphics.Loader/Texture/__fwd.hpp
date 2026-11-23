#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets.Type;
import Engine.Resource;
#endif

namespace hg::engine::graphics {
	//class TextureResource; -> Typedef
}

namespace hg::engine::graphics::loader {
	class TextureLoader;
	class TextureCache;
	class TextureFeedback;
	class TextureTransformer;
}

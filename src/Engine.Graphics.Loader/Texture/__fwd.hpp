#pragma once

#include <Engine.Resource/Loader/__fwd.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets.Type;
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

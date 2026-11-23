#pragma once

#include <Engine.Resource/Loader/__fwd.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets.Type;
#endif

namespace hg::engine::graphics {
	//class MaterialResource; -> Typedef
}

namespace hg::engine::graphics::loader {
	class MaterialLoader;
	class MaterialCache;
	class MaterialFeedback;
	class MaterialTransformer;

	class MaterialPrototypeLoader;
	class MaterialPrototypeCache;
	class MaterialPrototypeFeedback;
	class MaterialPrototypeTransformer;
}

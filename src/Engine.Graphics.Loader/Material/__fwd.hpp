#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets.Type;
import Engine.Resource;
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

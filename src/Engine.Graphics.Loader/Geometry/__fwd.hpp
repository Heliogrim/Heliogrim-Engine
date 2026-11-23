#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets.Type;
import Engine.Resource;
#endif

namespace hg::engine::graphics {
	//class StaticGeometryResource; -> Typedef
}

namespace hg::engine::graphics::loader {
	class StaticGeometryLoader;
	class StaticGeometryCache;
	class StaticGeometryFeedback;
	class StaticGeometryTransformer;
}

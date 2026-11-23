#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets.Type;
import Engine.Resource;
#endif

namespace hg::engine::accel {
	//class EffectResource; -> Typedef
}

namespace hg::engine::accel::loader {
	class EffectLoader;
	class EffectCache;
	class EffectFeedback;
	class EffectTransformer;
}

#pragma once

#include <Engine.Assets.Type/Material/GraphicsMaterial.hpp>
#include <Engine.Assets.Type/Material/GraphicsMaterialPrototype.hpp>
#include <Engine.Resource/Loader/__fwd.hpp>

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

#pragma once

#include <Engine.Assets.Type/Material/GfxMaterial.hpp>
#include <Engine.Assets.Type/Material/GfxMaterialPrototype.hpp>
#include <Engine.Resource/Loader/__fwd.hpp>

namespace hg::engine::gfx {
	//class MaterialResource; -> Typedef
}

namespace hg::engine::gfx::loader {
	class MaterialLoader;
	class MaterialCache;
	class MaterialFeedback;
	class MaterialTransformer;

	class MaterialPrototypeLoader;
	class MaterialPrototypeCache;
	class MaterialPrototypeFeedback;
	class MaterialPrototypeTransformer;
}

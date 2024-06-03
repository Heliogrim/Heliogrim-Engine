#pragma once
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
	class DefaultNormal :
		public ImageAsset,
		public AssetAutoGuid<DefaultNormal> {
	public:
		DefaultNormal():
			ImageAsset(
				clone(DefaultNormal::unstable_auto_guid()),
				R"(resources\imports\ktx\default_normal.ktx)"
			) {}
	};
}

#pragma once
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
	class DefaultRoughness :
		public ImageAsset,
		public AssetAutoGuid<DefaultRoughness> {
	public:
		DefaultRoughness():
			ImageAsset(
				clone(DefaultRoughness::unstable_auto_guid()),
				R"(resources\imports\ktx\default_roughness.ktx)"
			) {}
	};
}

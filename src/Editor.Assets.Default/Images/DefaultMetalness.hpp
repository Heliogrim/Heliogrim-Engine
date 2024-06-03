#pragma once
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
	class DefaultMetalness :
		public ImageAsset,
		public AssetAutoGuid<DefaultMetalness> {
	public:
		DefaultMetalness():
			ImageAsset(
				clone(DefaultMetalness::unstable_auto_guid()),
				R"(resources\imports\ktx\default_metalness.ktx)"
			) {}
	};
}

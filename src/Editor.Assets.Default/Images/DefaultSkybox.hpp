#pragma once
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
	class DefaultSkybox :
		public ImageAsset,
		public AssetAutoGuid<DefaultSkybox> {
	public:
		DefaultSkybox():
			ImageAsset(
				clone(DefaultSkybox::unstable_auto_guid()),
				R"(resources\imports\ktx\default_skybox.ktx)"
			) {}
	};
}

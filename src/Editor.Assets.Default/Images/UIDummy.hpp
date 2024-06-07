#pragma once
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
	class UIDummy :
		public ImageAsset,
		public AssetAutoGuid<UIDummy> {
	public:
		UIDummy():
			ImageAsset(
				clone(UIDummy::unstable_auto_guid()),
				R"(resources\imports\ktx\default_ui.ktx)"
			) {}
	};
}

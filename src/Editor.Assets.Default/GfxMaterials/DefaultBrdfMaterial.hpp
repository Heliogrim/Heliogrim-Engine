#pragma once
#include <Heliogrim/GfxMaterialAsset.hpp>

namespace hg::game::assets::material {
	class DefaultBrdfMaterial :
		public GfxMaterialAsset,
		public AssetAutoGuid<DefaultBrdfMaterial> {
	public:
		DefaultBrdfMaterial();
	};
}

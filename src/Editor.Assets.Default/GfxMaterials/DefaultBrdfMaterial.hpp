#pragma once

#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/Asset/Material/GfxMaterialAsset.hpp>

namespace hg::game::assets::material {
	class DefaultBrdfMaterial :
		public GfxMaterialAsset,
		public AssetAutoGuid<DefaultBrdfMaterial> {
	public:
		DefaultBrdfMaterial();
	};
}

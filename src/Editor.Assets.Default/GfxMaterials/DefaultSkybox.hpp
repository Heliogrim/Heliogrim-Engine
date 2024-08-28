#pragma once

#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/Asset/Material/GfxMaterialAsset.hpp>

namespace hg::game::assets::material {
	class DefaultSkybox :
		public GfxMaterialAsset,
		public AssetAutoGuid<DefaultSkybox> {
	public:
		DefaultSkybox();
	};
}

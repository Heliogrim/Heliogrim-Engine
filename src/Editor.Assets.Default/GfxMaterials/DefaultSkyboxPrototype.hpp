#pragma once

#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/Asset/Material/GfxMatProtoAsset.hpp>

namespace hg::game::assets::material {
	class DefaultSkyboxPrototype :
		public GfxMatProtoAsset,
		public AssetAutoGuid<DefaultSkyboxPrototype> {
	public:
		DefaultSkyboxPrototype();
	};
}

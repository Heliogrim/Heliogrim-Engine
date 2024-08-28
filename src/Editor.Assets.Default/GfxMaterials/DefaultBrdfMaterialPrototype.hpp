#pragma once

#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/Asset/Material/GfxMatProtoAsset.hpp>

namespace hg::game::assets::material {
	class DefaultBrdfMaterialPrototype :
		public GfxMatProtoAsset,
		public AssetAutoGuid<DefaultBrdfMaterialPrototype> {
	public:
		DefaultBrdfMaterialPrototype();
	};
}

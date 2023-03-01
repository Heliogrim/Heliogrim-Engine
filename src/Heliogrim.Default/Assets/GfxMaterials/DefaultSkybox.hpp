#pragma once
#include <Heliogrim/GfxMaterialAsset.hpp>

namespace hg::game::assets::material {
    class DefaultSkybox :
        public GfxMaterialAsset,
        public AssetAutoGuid<DefaultSkybox> {
    public:
        DefaultSkybox();
    };
}

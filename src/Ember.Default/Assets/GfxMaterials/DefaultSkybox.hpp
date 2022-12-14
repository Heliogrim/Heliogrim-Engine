#pragma once
#include <Ember/GfxMaterialAsset.hpp>

namespace ember::game::assets::material {
    class DefaultSkybox :
        public GfxMaterialAsset,
        public AssetAutoGuid<DefaultSkybox> {
    public:
        DefaultSkybox();
    };
}

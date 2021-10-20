#pragma once

#include <Ember/AssetAutoGuid.hpp>
#include <Ember/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace ember::game::assets::material {

    class GrassWild01 :
        public GfxMaterialAsset,
        public AssetAutoGuid<GrassWild01> {
    public:
        GrassWild01();
    };

    AUTO_REGISTER_ASSET(GrassWild01)
}

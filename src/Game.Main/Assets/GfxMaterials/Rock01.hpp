#pragma once

#include <Ember/AssetAutoGuid.hpp>
#include <Ember/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace ember::game::assets::material {

    class Rock01 :
        public GfxMaterialAsset,
        public AssetAutoGuid<Rock01> {
    public:
        Rock01();
    };

    AUTO_REGISTER_ASSET(Rock01)
}

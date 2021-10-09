#pragma once

#include <Ember/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace ember::game::assets::material {

    class GrassWild01 :
        public GfxMaterialAsset {
    public:
        static const asset_guid guid;

    public:
        GrassWild01();
    };
}

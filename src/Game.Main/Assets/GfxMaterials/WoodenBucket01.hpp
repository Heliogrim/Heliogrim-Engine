#pragma once

#include <Ember/AssetAutoGuid.hpp>
#include <Ember/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace ember::game::assets::material {

    class WoodenBucket01 :
        public GfxMaterialAsset,
        public AssetAutoGuid<WoodenBucket01> {
    public:
        WoodenBucket01();
    };

    AUTO_REGISTER_ASSET(WoodenBucket01)
}

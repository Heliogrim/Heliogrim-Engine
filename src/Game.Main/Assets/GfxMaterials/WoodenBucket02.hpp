#pragma once

#include <Ember/AssetAutoGuid.hpp>
#include <Ember/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace ember::game::assets::material {

    class WoodenBucket02 :
        public GfxMaterialAsset,
        public AssetAutoGuid<WoodenBucket02> {
    public:
        WoodenBucket02();
    };

    AUTO_REGISTER_ASSET(WoodenBucket02)
}

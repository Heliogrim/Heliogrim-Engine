#pragma once

#include <Ember/AssetAutoGuid.hpp>
#include <Ember/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace ember::game::assets::material {

    class WoodenPier01Planks :
        public GfxMaterialAsset,
        public AssetAutoGuid<WoodenPier01Planks> {
    public:
        WoodenPier01Planks();
    };

    AUTO_REGISTER_ASSET(WoodenPier01Planks)
}

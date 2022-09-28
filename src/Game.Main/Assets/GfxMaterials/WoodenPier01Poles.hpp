#pragma once

#include <Ember/AssetAutoGuid.hpp>
#include <Ember/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace ember::game::assets::material {

    class WoodenPier01Poles :
        public GfxMaterialAsset,
        public AssetAutoGuid<WoodenPier01Poles> {
    public:
        WoodenPier01Poles();
    };

    AUTO_REGISTER_ASSET(WoodenPier01Poles)
}

#pragma once

#include <Ember/AssetAutoGuid.hpp>
#include <Ember/GfxMaterialAsset.hpp>

namespace ember::game::assets::material {
    class DryGroundRocks01 :
        public GfxMaterialAsset,
        public AssetAutoGuid<DryGroundRocks01> {
    public:
        DryGroundRocks01();
    };
}

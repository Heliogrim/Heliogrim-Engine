#pragma once

#include <Ember/AssetAutoGuid.hpp>
#include <Ember/GfxMaterialAsset.hpp>

namespace ember::game::assets::material {
    class ForestGround01 :
        public GfxMaterialAsset,
        public AssetAutoGuid<ForestGround01> {
    public:
        ForestGround01();
    };
}

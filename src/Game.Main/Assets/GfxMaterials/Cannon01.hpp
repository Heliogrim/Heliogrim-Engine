#pragma once

#include <Ember/AssetAutoGuid.hpp>
#include <Ember/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace ember::game::assets::material {
    class Cannon01 :
        public GfxMaterialAsset,
        public AssetAutoGuid<Cannon01> {
    public:
        Cannon01();
    };
}

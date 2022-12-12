#pragma once

#include <Ember/AssetAutoGuid.hpp>
#include <Ember/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace ember::game::assets::material {
    class Dandelion01 :
        public GfxMaterialAsset,
        public AssetAutoGuid<Dandelion01> {
    public:
        Dandelion01();
    };
}

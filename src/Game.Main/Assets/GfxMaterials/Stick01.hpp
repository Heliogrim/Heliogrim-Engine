#pragma once

#include <Ember/AssetAutoGuid.hpp>
#include <Ember/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace ember::game::assets::material {
    class Stick01 :
        public GfxMaterialAsset,
        public AssetAutoGuid<Stick01> {
    public:
        Stick01();
    };
}

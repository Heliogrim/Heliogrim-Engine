#pragma once

#include <Ember/AssetAutoGuid.hpp>
#include <Ember/GfxMaterialAsset.hpp>
#include <Engine.Assets/AssetGuid.hpp>

namespace ember::game::assets::material {
    class Cerberus :
        public GfxMaterialAsset,
        public AssetAutoGuid<Cerberus> {
    public:
        Cerberus();
    };
}

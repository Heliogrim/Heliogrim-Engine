#pragma once

#include <Ember/AssetAutoGuid.hpp>
#include <Ember/GfxMaterialAsset.hpp>

namespace ember::game::assets::material {
    class Skybox01 :
        public GfxMaterialAsset,
        public AssetAutoGuid<Skybox01> {
    public:
        Skybox01();
    };
}

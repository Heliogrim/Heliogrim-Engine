#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class Cannon01Roughness :
        public TextureAsset,
        public AssetAutoGuid<Cannon01Roughness> {
    public:
        Cannon01Roughness();
    };
}

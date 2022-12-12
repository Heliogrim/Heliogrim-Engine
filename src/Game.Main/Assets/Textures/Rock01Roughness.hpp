#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class Rock01Roughness :
        public TextureAsset,
        public AssetAutoGuid<Rock01Roughness> {
    public:
        Rock01Roughness();
    };
}

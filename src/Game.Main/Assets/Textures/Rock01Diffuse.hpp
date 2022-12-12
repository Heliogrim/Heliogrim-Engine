#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class Rock01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<Rock01Diffuse> {
    public:
        Rock01Diffuse();
    };
}

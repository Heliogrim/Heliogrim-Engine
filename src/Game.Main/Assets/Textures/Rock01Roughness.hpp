#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class Rock01Roughness :
        public TextureAsset,
        public AssetAutoGuid<Rock01Roughness> {
    public:
        Rock01Roughness();
    };

    AUTO_REGISTER_ASSET(Rock01Roughness)

}

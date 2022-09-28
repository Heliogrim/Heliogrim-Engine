#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class Rock01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<Rock01Diffuse> {
    public:
        Rock01Diffuse();
    };

    AUTO_REGISTER_ASSET(Rock01Diffuse)

}

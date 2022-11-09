#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class Cannon01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<Cannon01Diffuse> {
    public:
        Cannon01Diffuse();
    };

    AUTO_REGISTER_ASSET(Cannon01Diffuse)

}

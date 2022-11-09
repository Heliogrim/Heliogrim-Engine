#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class Cannon01Roughness :
        public TextureAsset,
        public AssetAutoGuid<Cannon01Roughness> {
    public:
        Cannon01Roughness();
    };

    AUTO_REGISTER_ASSET(Cannon01Roughness)

}

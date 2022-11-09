#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class Cannon01Metalness :
        public TextureAsset,
        public AssetAutoGuid<Cannon01Metalness> {
    public:
        Cannon01Metalness();
    };

    AUTO_REGISTER_ASSET(Cannon01Metalness)

}

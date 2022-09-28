#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class Stick01AO :
        public TextureAsset,
        public AssetAutoGuid<Stick01AO> {
    public:
        Stick01AO();
    };

    AUTO_REGISTER_ASSET(Stick01AO)

}

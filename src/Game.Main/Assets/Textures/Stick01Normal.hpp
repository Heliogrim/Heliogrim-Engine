#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class Stick01Normal :
        public TextureAsset,
        public AssetAutoGuid<Stick01Normal> {
    public:
        Stick01Normal();
    };

    AUTO_REGISTER_ASSET(Stick01Normal)

}

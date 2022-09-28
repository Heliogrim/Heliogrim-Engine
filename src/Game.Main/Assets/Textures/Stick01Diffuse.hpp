#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class Stick01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<Stick01Diffuse> {
    public:
        Stick01Diffuse();
    };

    AUTO_REGISTER_ASSET(Stick01Diffuse)

}

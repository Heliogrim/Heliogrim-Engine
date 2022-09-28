#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class Dandelion01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<Dandelion01Diffuse> {
    public:
        Dandelion01Diffuse();
    };

    AUTO_REGISTER_ASSET(Dandelion01Diffuse)

}

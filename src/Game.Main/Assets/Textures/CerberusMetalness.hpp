#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class CerberusMetalness :
        public TextureAsset,
        public AssetAutoGuid<CerberusMetalness> {
    public:
        CerberusMetalness();
    };

    AUTO_REGISTER_ASSET(CerberusMetalness)

}

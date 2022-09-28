#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class CerberusRoughness :
        public TextureAsset,
        public AssetAutoGuid<CerberusRoughness> {
    public:
        CerberusRoughness();
    };

    AUTO_REGISTER_ASSET(CerberusRoughness)

}

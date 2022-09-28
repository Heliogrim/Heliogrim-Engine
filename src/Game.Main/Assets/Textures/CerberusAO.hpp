#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class CerberusAO :
        public TextureAsset,
        public AssetAutoGuid<CerberusAO> {
    public:
        CerberusAO();
    };

    AUTO_REGISTER_ASSET(CerberusAO)

}

#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class CerberusDiffuse :
        public TextureAsset,
        public AssetAutoGuid<CerberusDiffuse> {
    public:
        CerberusDiffuse();
    };

    AUTO_REGISTER_ASSET(CerberusDiffuse)

}

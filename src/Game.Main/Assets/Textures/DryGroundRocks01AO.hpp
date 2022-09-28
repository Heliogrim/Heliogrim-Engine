#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class DryGroundRocks01AO :
        public TextureAsset,
        public AssetAutoGuid<DryGroundRocks01AO> {
    public:
        DryGroundRocks01AO();
    };

    AUTO_REGISTER_ASSET(DryGroundRocks01AO)

}

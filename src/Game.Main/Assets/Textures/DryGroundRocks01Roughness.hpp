#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class DryGroundRocks01Roughness :
        public TextureAsset,
        public AssetAutoGuid<DryGroundRocks01Roughness> {
    public:
        DryGroundRocks01Roughness();
    };

    AUTO_REGISTER_ASSET(DryGroundRocks01Roughness)

}

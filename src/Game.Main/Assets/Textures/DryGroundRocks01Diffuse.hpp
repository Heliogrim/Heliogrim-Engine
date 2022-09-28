#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class DryGroundRocks01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<DryGroundRocks01Diffuse> {
    public:
        DryGroundRocks01Diffuse();
    };

    AUTO_REGISTER_ASSET(DryGroundRocks01Diffuse)

}

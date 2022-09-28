#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class DryGroundRocks01Normal :
        public TextureAsset,
        public AssetAutoGuid<DryGroundRocks01Normal> {
    public:
        DryGroundRocks01Normal();
    };

    AUTO_REGISTER_ASSET(DryGroundRocks01Normal)

}

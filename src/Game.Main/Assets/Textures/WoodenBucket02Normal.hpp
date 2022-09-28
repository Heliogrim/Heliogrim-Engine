#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class WoodenBucket02Normal :
        public TextureAsset,
        public AssetAutoGuid<WoodenBucket02Normal> {
    public:
        WoodenBucket02Normal();
    };

    AUTO_REGISTER_ASSET(WoodenBucket02Normal)

}

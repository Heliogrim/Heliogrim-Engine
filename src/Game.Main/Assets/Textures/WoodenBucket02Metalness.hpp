#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class WoodenBucket02Metalness :
        public TextureAsset,
        public AssetAutoGuid<WoodenBucket02Metalness> {
    public:
        WoodenBucket02Metalness();
    };

    AUTO_REGISTER_ASSET(WoodenBucket02Metalness)

}

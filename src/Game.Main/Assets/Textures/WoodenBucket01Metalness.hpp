#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class WoodenBucket01Metalness :
        public TextureAsset,
        public AssetAutoGuid<WoodenBucket01Metalness> {
    public:
        WoodenBucket01Metalness();
    };

    AUTO_REGISTER_ASSET(WoodenBucket01Metalness)

}

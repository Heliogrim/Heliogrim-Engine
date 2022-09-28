#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class WoodenBucket01Normal :
        public TextureAsset,
        public AssetAutoGuid<WoodenBucket01Normal> {
    public:
        WoodenBucket01Normal();
    };

    AUTO_REGISTER_ASSET(WoodenBucket01Normal)

}

#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class WoodenBucket01Roughness :
        public TextureAsset,
        public AssetAutoGuid<WoodenBucket01Roughness> {
    public:
        WoodenBucket01Roughness();
    };

    AUTO_REGISTER_ASSET(WoodenBucket01Roughness)

}

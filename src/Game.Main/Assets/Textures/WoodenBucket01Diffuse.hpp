#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class WoodenBucket01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<WoodenBucket01Diffuse> {
    public:
        WoodenBucket01Diffuse();
    };

    AUTO_REGISTER_ASSET(WoodenBucket01Diffuse)

}

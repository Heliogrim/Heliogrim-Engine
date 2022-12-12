#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class WoodenBucket02Roughness :
        public TextureAsset,
        public AssetAutoGuid<WoodenBucket02Roughness> {
    public:
        WoodenBucket02Roughness();
    };
}

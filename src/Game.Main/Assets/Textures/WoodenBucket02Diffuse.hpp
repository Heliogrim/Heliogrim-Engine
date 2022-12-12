#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class WoodenBucket02Diffuse :
        public TextureAsset,
        public AssetAutoGuid<WoodenBucket02Diffuse> {
    public:
        WoodenBucket02Diffuse();
    };
}

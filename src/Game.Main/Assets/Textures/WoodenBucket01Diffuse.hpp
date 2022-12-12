#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class WoodenBucket01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<WoodenBucket01Diffuse> {
    public:
        WoodenBucket01Diffuse();
    };
}

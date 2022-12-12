#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class WoodenBucket01Metalness :
        public TextureAsset,
        public AssetAutoGuid<WoodenBucket01Metalness> {
    public:
        WoodenBucket01Metalness();
    };
}

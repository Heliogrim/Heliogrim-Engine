#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class WoodenBucket01Normal :
        public TextureAsset,
        public AssetAutoGuid<WoodenBucket01Normal> {
    public:
        WoodenBucket01Normal();
    };
}

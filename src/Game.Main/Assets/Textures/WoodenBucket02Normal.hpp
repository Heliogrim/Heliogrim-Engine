#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class WoodenBucket02Normal :
        public TextureAsset,
        public AssetAutoGuid<WoodenBucket02Normal> {
    public:
        WoodenBucket02Normal();
    };
}

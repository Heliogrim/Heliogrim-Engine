#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class WoodenBucket02Metalness :
        public TextureAsset,
        public AssetAutoGuid<WoodenBucket02Metalness> {
    public:
        WoodenBucket02Metalness();
    };
}

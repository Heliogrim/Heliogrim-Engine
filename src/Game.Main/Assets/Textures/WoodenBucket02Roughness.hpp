#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class WoodenBucket02Roughness :
        public TextureAsset,
        public AssetAutoGuid<WoodenBucket02Roughness> {
    public:
        WoodenBucket02Roughness();
    };
}

#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class WoodenBucket01Roughness :
        public TextureAsset,
        public AssetAutoGuid<WoodenBucket01Roughness> {
    public:
        WoodenBucket01Roughness();
    };
}

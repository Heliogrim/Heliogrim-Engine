#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class WoodenBucket01Metalness :
        public TextureAsset,
        public AssetAutoGuid<WoodenBucket01Metalness> {
    public:
        WoodenBucket01Metalness();
    };
}

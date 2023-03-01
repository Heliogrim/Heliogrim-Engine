#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class WoodenBucket02Normal :
        public TextureAsset,
        public AssetAutoGuid<WoodenBucket02Normal> {
    public:
        WoodenBucket02Normal();
    };
}

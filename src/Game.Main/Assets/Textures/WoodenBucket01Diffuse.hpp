#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class WoodenBucket01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<WoodenBucket01Diffuse> {
    public:
        WoodenBucket01Diffuse();
    };
}

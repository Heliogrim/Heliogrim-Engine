#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class WoodenBucket02Diffuse :
        public TextureAsset,
        public AssetAutoGuid<WoodenBucket02Diffuse> {
    public:
        WoodenBucket02Diffuse();
    };
}

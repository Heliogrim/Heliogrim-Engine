#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class WoodenBucket02Roughness :
        public ImageAsset,
        public AssetAutoGuid<WoodenBucket02Roughness> {
    public:
        WoodenBucket02Roughness():
            ImageAsset(WoodenBucket02Roughness::unstable_auto_guid(),
                R"(resources\imports\ktx2\wooden_bucket_02_roughness.ktx2)") {}
    };
}

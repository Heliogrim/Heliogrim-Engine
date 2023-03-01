#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class WoodenBucket02Normal :
        public ImageAsset,
        public AssetAutoGuid<WoodenBucket02Normal> {
    public:
        WoodenBucket02Normal():
            ImageAsset(WoodenBucket02Normal::unstable_auto_guid(),
                R"(resources\imports\ktx2\wooden_bucket_02_normal.ktx2)") {}
    };
}

#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class WoodenBucket01Normal :
        public ImageAsset,
        public AssetAutoGuid<WoodenBucket01Normal> {
    public:
        WoodenBucket01Normal():
            ImageAsset(WoodenBucket01Normal::unstable_auto_guid(),
                R"(resources\imports\ktx2\wooden_bucket_01_normal.ktx2)") {}
    };
}

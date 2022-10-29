#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class WoodenBucket02Normal:
        public ImageAsset,
        public AssetAutoGuid<WoodenBucket02Normal> {
    public:
        WoodenBucket02Normal():
            ImageAsset(WoodenBucket02Normal::auto_guid(),
                R"(resources\imports\ktx2\wooden_bucket_02_normal.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(WoodenBucket02Normal)
}

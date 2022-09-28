#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class WoodenBucket01Normal:
        public ImageAsset,
        public AssetAutoGuid<WoodenBucket01Normal> {
    public:
        WoodenBucket01Normal():
            ImageAsset(WoodenBucket01Normal::auto_guid(),
                R"(resources\assets\texture\wooden_bucket_01_normal.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(WoodenBucket01Normal)
}

#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class WoodenBucket02Metalness:
        public ImageAsset,
        public AssetAutoGuid<WoodenBucket02Metalness> {
    public:
        WoodenBucket02Metalness():
            ImageAsset(WoodenBucket02Metalness::unstable_auto_guid(),
                R"(resources\imports\ktx2\wooden_bucket_02_metalness.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(WoodenBucket02Metalness)
}

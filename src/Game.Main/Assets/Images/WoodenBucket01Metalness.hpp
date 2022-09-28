#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class WoodenBucket01Metalness:
        public ImageAsset,
        public AssetAutoGuid<WoodenBucket01Metalness> {
    public:
        WoodenBucket01Metalness():
            ImageAsset(WoodenBucket01Metalness::auto_guid(),
                R"(resources\assets\texture\wooden_bucket_01_metalness.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(WoodenBucket01Metalness)
}

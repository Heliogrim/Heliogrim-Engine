#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class WoodenBucket01Roughness:
        public ImageAsset,
        public AssetAutoGuid<WoodenBucket01Roughness> {
    public:
        WoodenBucket01Roughness():
            ImageAsset(WoodenBucket01Roughness::auto_guid(),
                R"(resources\assets\texture\wooden_bucket_01_roughness.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(WoodenBucket01Roughness)
}

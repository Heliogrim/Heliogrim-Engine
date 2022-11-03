#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class WoodenBucket02Roughness:
        public ImageAsset,
        public AssetAutoGuid<WoodenBucket02Roughness> {
    public:
        WoodenBucket02Roughness():
            ImageAsset(WoodenBucket02Roughness::unstable_auto_guid(),
                R"(resources\imports\ktx2\wooden_bucket_02_roughness.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(WoodenBucket02Roughness)
}

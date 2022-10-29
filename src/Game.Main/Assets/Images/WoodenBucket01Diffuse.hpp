#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class WoodenBucket01Diffuse:
        public ImageAsset,
        public AssetAutoGuid<WoodenBucket01Diffuse> {
    public:
        WoodenBucket01Diffuse():
            ImageAsset(WoodenBucket01Diffuse::auto_guid(),
                R"(resources\imports\ktx2\wooden_bucket_01_diffuse.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(WoodenBucket01Diffuse)
}

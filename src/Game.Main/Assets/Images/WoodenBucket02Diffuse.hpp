#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class WoodenBucket02Diffuse:
        public ImageAsset,
        public AssetAutoGuid<WoodenBucket02Diffuse> {
    public:
        WoodenBucket02Diffuse():
            ImageAsset(WoodenBucket02Diffuse::auto_guid(),
                R"(resources\assets\texture\wooden_bucket_02_diffuse.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(WoodenBucket02Diffuse)
}

#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class WoodenBucket02Diffuse :
        public ImageAsset,
        public AssetAutoGuid<WoodenBucket02Diffuse> {
    public:
        WoodenBucket02Diffuse():
            ImageAsset(WoodenBucket02Diffuse::unstable_auto_guid(),
                R"(resources\imports\ktx2\wooden_bucket_02_diffuse.ktx2)") {}
    };
}

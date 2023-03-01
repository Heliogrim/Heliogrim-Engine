#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class WoodenBucket01Diffuse :
        public ImageAsset,
        public AssetAutoGuid<WoodenBucket01Diffuse> {
    public:
        WoodenBucket01Diffuse():
            ImageAsset(WoodenBucket01Diffuse::unstable_auto_guid(),
                R"(resources\imports\ktx2\wooden_bucket_01_diffuse.ktx2)") {}
    };
}

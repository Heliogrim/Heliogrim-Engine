#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class WoodenBucket01Metalness :
        public ImageAsset,
        public AssetAutoGuid<WoodenBucket01Metalness> {
    public:
        WoodenBucket01Metalness():
            ImageAsset(WoodenBucket01Metalness::unstable_auto_guid(),
                R"(resources\imports\ktx2\wooden_bucket_01_metalness.ktx2)") {}
    };
}

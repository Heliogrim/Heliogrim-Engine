#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class WoodenBucket01Roughness :
        public ImageAsset,
        public AssetAutoGuid<WoodenBucket01Roughness> {
    public:
        WoodenBucket01Roughness():
            ImageAsset(WoodenBucket01Roughness::unstable_auto_guid(),
                R"(resources\imports\ktx2\wooden_bucket_01_roughness.ktx2)") {}
    };
}

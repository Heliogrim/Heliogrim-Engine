#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class ForestGround01Normal :
        public ImageAsset,
        public AssetAutoGuid<ForestGround01Normal> {
    public:
        ForestGround01Normal():
            ImageAsset(ForestGround01Normal::unstable_auto_guid(),
                R"(resources\imports\ktx2\forest_ground_01_normal.ktx2)") {}
    };
}

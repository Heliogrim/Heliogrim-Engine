#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class ForestGround01AO :
        public ImageAsset,
        public AssetAutoGuid<ForestGround01AO> {
    public:
        ForestGround01AO():
            ImageAsset(ForestGround01AO::unstable_auto_guid(),
                R"(resources\imports\ktx2\forest_ground_01_ao.ktx2)") {}
    };
}

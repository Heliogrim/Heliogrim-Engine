#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class ForestGround01Normal :
        public ImageAsset,
        public AssetAutoGuid<ForestGround01Normal> {
    public:
        ForestGround01Normal():
            ImageAsset(ForestGround01Normal::unstable_auto_guid(),
                R"(resources\imports\ktx2\forest_ground_01_normal.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(ForestGround01Normal)
}

#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class ForestGround01AO :
        public ImageAsset,
        public AssetAutoGuid<ForestGround01AO> {
    public:
        ForestGround01AO():
            ImageAsset(ForestGround01AO::auto_guid(),
                R"(resources\imports\ktx2\forest_ground_01_ao.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(ForestGround01AO)
}

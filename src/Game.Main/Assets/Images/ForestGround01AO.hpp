#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class ForestGround01AO :
        public ImageAsset,
        public AssetAutoGuid<ForestGround01AO> {
    public:
        ForestGround01AO():
            ImageAsset(ForestGround01AO::auto_guid(),
                R"(resources\assets\texture\forest_ground_01_ao.ktx)") {}
    };

    AUTO_REGISTER_ASSET(ForestGround01AO)
}

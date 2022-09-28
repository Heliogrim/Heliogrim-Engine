#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class ForestGround01Roughness :
        public ImageAsset,
        public AssetAutoGuid<ForestGround01Roughness> {
    public:
        ForestGround01Roughness():
            ImageAsset(ForestGround01Roughness::auto_guid(),
                R"(resources\assets\texture\forest_ground_01_roughness.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(ForestGround01Roughness)
}

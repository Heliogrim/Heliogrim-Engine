#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class ForestGround01Diffuse :
        public ImageAsset,
        public AssetAutoGuid<ForestGround01Diffuse> {
    public:
        ForestGround01Diffuse():
            ImageAsset(ForestGround01Diffuse::auto_guid(),
                R"(resources\assets\texture\forest_ground_01_diffuse.ktx)") {}
    };

    AUTO_REGISTER_ASSET(ForestGround01Diffuse)
}

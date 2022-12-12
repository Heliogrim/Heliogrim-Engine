#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class ForestGround01Roughness :
        public ImageAsset,
        public AssetAutoGuid<ForestGround01Roughness> {
    public:
        ForestGround01Roughness():
            ImageAsset(ForestGround01Roughness::unstable_auto_guid(),
                R"(resources\imports\ktx2\forest_ground_01_roughness.ktx2)") {}
    };
}

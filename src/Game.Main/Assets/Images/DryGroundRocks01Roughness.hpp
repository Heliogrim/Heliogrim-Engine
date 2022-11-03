#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class DryGroundRocks01Roughness :
        public ImageAsset,
        public AssetAutoGuid<DryGroundRocks01Roughness> {
    public:
        DryGroundRocks01Roughness():
            ImageAsset(DryGroundRocks01Roughness::unstable_auto_guid(),
                R"(resources\imports\ktx2\dry_ground_rocks_01_roughness.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(DryGroundRocks01Roughness)
}

#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class DryGroundRocks01Normal :
        public ImageAsset,
        public AssetAutoGuid<DryGroundRocks01Normal> {
    public:
        DryGroundRocks01Normal():
            ImageAsset(DryGroundRocks01Normal::unstable_auto_guid(),
                R"(resources\imports\ktx2\dry_ground_rocks_01_normal.ktx2)") {}
    };
}

#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class DryGroundRocks01AO :
        public ImageAsset,
        public AssetAutoGuid<DryGroundRocks01AO> {
    public:
        DryGroundRocks01AO():
            ImageAsset(DryGroundRocks01AO::unstable_auto_guid(),
                R"(resources\imports\ktx2\dry_ground_rocks_01_ao.ktx2)") {}
    };
}

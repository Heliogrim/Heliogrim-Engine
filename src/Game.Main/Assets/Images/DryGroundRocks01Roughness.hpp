#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class DryGroundRocks01Roughness :
        public ImageAsset,
        public AssetAutoGuid<DryGroundRocks01Roughness> {
    public:
        DryGroundRocks01Roughness():
            ImageAsset(DryGroundRocks01Roughness::unstable_auto_guid(),
                R"(resources\imports\ktx2\dry_ground_rocks_01_roughness.ktx2)") {}
    };
}

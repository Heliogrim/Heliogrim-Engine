#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class DryGroundRocks01Diffuse :
        public ImageAsset,
        public AssetAutoGuid<DryGroundRocks01Diffuse> {
    public:
        DryGroundRocks01Diffuse():
            ImageAsset(DryGroundRocks01Diffuse::unstable_auto_guid(),
                R"(resources\imports\ktx2\dry_ground_rocks_01_diffuse.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(DryGroundRocks01Diffuse)
}

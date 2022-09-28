#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class DryGroundRocks01AO :
        public ImageAsset,
        public AssetAutoGuid<DryGroundRocks01AO> {
    public:
        DryGroundRocks01AO():
            ImageAsset(DryGroundRocks01AO::auto_guid(),
                R"(resources\assets\texture\dry_ground_rocks_01_ao.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(DryGroundRocks01AO)
}

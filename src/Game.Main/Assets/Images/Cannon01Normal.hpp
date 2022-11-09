#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class Cannon01Normal:
        public ImageAsset,
        public AssetAutoGuid<Cannon01Normal> {
    public:
        Cannon01Normal():
            ImageAsset(Cannon01Normal::unstable_auto_guid(),
                R"(resources\imports\ktx2\cannon_01_normal.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(Cannon01Normal)
}

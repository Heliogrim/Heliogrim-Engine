#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class Cannon01Metalness:
        public ImageAsset,
        public AssetAutoGuid<Cannon01Metalness> {
    public:
        Cannon01Metalness():
            ImageAsset(Cannon01Metalness::unstable_auto_guid(),
                R"(resources\imports\ktx2\cannon_01_metalness.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(Cannon01Metalness)
}

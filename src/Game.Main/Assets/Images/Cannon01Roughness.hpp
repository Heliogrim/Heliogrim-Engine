#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class Cannon01Roughness :
        public ImageAsset,
        public AssetAutoGuid<Cannon01Roughness> {
    public:
        Cannon01Roughness():
            ImageAsset(Cannon01Roughness::unstable_auto_guid(),
                R"(resources\imports\ktx2\cannon_01_roughness.ktx2)") {}
    };
}

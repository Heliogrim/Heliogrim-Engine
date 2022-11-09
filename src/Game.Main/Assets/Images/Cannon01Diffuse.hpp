#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class Cannon01Diffuse:
        public ImageAsset,
        public AssetAutoGuid<Cannon01Diffuse> {
    public:
        Cannon01Diffuse():
            ImageAsset(Cannon01Diffuse::unstable_auto_guid(),
                R"(resources\imports\ktx2\cannon_01_diffuse.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(Cannon01Diffuse)
}

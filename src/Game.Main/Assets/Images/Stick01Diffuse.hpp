#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class Stick01Diffuse :
        public ImageAsset,
        public AssetAutoGuid<Stick01Diffuse> {
    public:
        Stick01Diffuse():
            ImageAsset(Stick01Diffuse::unstable_auto_guid(),
                R"(resources\imports\ktx2\stick_01_diffuse.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(Stick01Diffuse)
}

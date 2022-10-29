#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class Stick01AO :
        public ImageAsset,
        public AssetAutoGuid<Stick01AO> {
    public:
        Stick01AO():
            ImageAsset(Stick01AO::auto_guid(),
                R"(resources\imports\ktx2\stick_01_ao.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(Stick01AO)
}

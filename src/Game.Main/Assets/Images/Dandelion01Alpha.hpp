#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class Dandelion01Alpha :
        public ImageAsset,
        public AssetAutoGuid<Dandelion01Alpha> {
    public:
        Dandelion01Alpha():
            ImageAsset(Dandelion01Alpha::unstable_auto_guid(),
                R"(resources\assets\texture\dandelion_01_alpha.ktx2)") {}
    };
}
#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class Dandelion01Normal :
        public ImageAsset,
        public AssetAutoGuid<Dandelion01Normal> {
    public:
        Dandelion01Normal():
            ImageAsset(Dandelion01Normal::unstable_auto_guid(),
                R"(resources\assets\texture\dandelion_01_normal.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(Dandelion01Normal)
}

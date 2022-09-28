#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class WoodenPier01PolesDiffuse :
        public ImageAsset,
        public AssetAutoGuid<WoodenPier01PolesDiffuse> {
    public:
        WoodenPier01PolesDiffuse():
            ImageAsset(WoodenPier01PolesDiffuse::auto_guid(),
                R"(resources\assets\texture\wooden_pier_01_poles_diffuse.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(WoodenPier01PolesDiffuse)
}

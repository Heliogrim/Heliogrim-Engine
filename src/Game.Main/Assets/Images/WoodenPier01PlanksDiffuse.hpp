#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class WoodenPier01PlanksDiffuse :
        public ImageAsset,
        public AssetAutoGuid<WoodenPier01PlanksDiffuse> {
    public:
        WoodenPier01PlanksDiffuse():
            ImageAsset(WoodenPier01PlanksDiffuse::auto_guid(),
                R"(resources\assets\texture\wooden_pier_01_planks_diffuse.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(WoodenPier01PlanksDiffuse)
}

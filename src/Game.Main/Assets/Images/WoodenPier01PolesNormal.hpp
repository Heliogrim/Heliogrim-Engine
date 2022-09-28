#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class WoodenPier01PolesNormal :
        public ImageAsset,
        public AssetAutoGuid<WoodenPier01PolesNormal> {
    public:
        WoodenPier01PolesNormal():
            ImageAsset(WoodenPier01PolesNormal::auto_guid(),
                R"(resources\assets\texture\wooden_pier_01_poles_normal.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(WoodenPier01PolesNormal)
}

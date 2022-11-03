#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class WoodenPier01PolesNormal :
        public ImageAsset,
        public AssetAutoGuid<WoodenPier01PolesNormal> {
    public:
        WoodenPier01PolesNormal():
            ImageAsset(WoodenPier01PolesNormal::unstable_auto_guid(),
                R"(resources\imports\ktx2\wooden_pier_01_poles_normal.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(WoodenPier01PolesNormal)
}

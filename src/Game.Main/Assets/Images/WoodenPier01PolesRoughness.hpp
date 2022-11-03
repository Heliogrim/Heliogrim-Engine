#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class WoodenPier01PolesRoughness:
        public ImageAsset,
        public AssetAutoGuid<WoodenPier01PolesRoughness> {
    public:
        WoodenPier01PolesRoughness():
            ImageAsset(WoodenPier01PolesRoughness::unstable_auto_guid(),
                R"(resources\imports\ktx2\wooden_pier_01_poles_roughness.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(WoodenPier01PolesRoughness)
}

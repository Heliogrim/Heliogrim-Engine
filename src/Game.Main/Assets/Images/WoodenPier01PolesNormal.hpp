#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class WoodenPier01PolesNormal :
        public ImageAsset,
        public AssetAutoGuid<WoodenPier01PolesNormal> {
    public:
        WoodenPier01PolesNormal():
            ImageAsset(WoodenPier01PolesNormal::unstable_auto_guid(),
                R"(resources\imports\ktx2\wooden_pier_01_poles_normal.ktx2)") {}
    };
}

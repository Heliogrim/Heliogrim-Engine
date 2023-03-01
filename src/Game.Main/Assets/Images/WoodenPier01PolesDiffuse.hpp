#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class WoodenPier01PolesDiffuse :
        public ImageAsset,
        public AssetAutoGuid<WoodenPier01PolesDiffuse> {
    public:
        WoodenPier01PolesDiffuse():
            ImageAsset(WoodenPier01PolesDiffuse::unstable_auto_guid(),
                R"(resources\imports\ktx2\wooden_pier_01_poles_diffuse.ktx2)") {}
    };
}

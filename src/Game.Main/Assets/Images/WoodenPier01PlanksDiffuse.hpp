#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class WoodenPier01PlanksDiffuse :
        public ImageAsset,
        public AssetAutoGuid<WoodenPier01PlanksDiffuse> {
    public:
        WoodenPier01PlanksDiffuse():
            ImageAsset(WoodenPier01PlanksDiffuse::unstable_auto_guid(),
                R"(resources\imports\ktx2\wooden_pier_01_planks_diffuse.ktx2)") {}
    };
}

#pragma once
#include <Heliogrim/ImageAsset.hpp>

#include "WoodenPier01PolesRoughness.hpp"

namespace hg::game::assets::image {
    class WoodenPier01PlanksRoughness :
        public ImageAsset,
        public AssetAutoGuid<WoodenPier01PlanksRoughness> {
    public:
        WoodenPier01PlanksRoughness():
            ImageAsset(WoodenPier01PlanksRoughness::unstable_auto_guid(),
                R"(resources\imports\ktx2\wooden_pier_01_planks_roughness.ktx2)") {}
    };
}

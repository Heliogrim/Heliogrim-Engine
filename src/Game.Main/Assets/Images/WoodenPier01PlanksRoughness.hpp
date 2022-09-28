#pragma once
#include <Ember/ImageAsset.hpp>

#include "WoodenPier01PolesRoughness.hpp"

namespace ember::game::assets::image {

    class WoodenPier01PlanksRoughness:
        public ImageAsset,
        public AssetAutoGuid<WoodenPier01PlanksRoughness> {
    public:
        WoodenPier01PlanksRoughness():
            ImageAsset(WoodenPier01PlanksRoughness::auto_guid(),
                R"(resources\assets\texture\wooden_pier_01_planks_roughness.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(WoodenPier01PlanksRoughness)
}

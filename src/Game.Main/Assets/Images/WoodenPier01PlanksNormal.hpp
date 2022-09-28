#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class WoodenPier01PlanksNormal :
        public ImageAsset,
        public AssetAutoGuid<WoodenPier01PlanksNormal> {
    public:
        WoodenPier01PlanksNormal():
            ImageAsset(WoodenPier01PlanksNormal::auto_guid(),
                R"(resources\assets\texture\wooden_pier_01_planks_normal.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(WoodenPier01PlanksNormal)
}

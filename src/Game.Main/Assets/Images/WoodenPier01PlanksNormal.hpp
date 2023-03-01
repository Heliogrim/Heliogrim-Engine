#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class WoodenPier01PlanksNormal :
        public ImageAsset,
        public AssetAutoGuid<WoodenPier01PlanksNormal> {
    public:
        WoodenPier01PlanksNormal():
            ImageAsset(WoodenPier01PlanksNormal::unstable_auto_guid(),
                R"(resources\imports\ktx2\wooden_pier_01_planks_normal.ktx2)") {}
    };
}

#pragma once
#include <Heliogrim/AssetAutoGuid.hpp>
#include <Heliogrim/TextureAsset.hpp>

namespace hg::game::assets::texture {
    class WoodenPier01PlanksNormal :
        public TextureAsset,
        public AssetAutoGuid<WoodenPier01PlanksNormal> {
    public:
        WoodenPier01PlanksNormal();
    };
}

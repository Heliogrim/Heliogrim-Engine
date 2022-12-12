#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class WoodenPier01PlanksNormal :
        public TextureAsset,
        public AssetAutoGuid<WoodenPier01PlanksNormal> {
    public:
        WoodenPier01PlanksNormal();
    };
}

#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class WoodenPier01PlanksNormal :
        public TextureAsset,
        public AssetAutoGuid<WoodenPier01PlanksNormal> {
    public:
        WoodenPier01PlanksNormal();
    };

    AUTO_REGISTER_ASSET(WoodenPier01PlanksNormal)

}

#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class WoodenPier01PlanksRoughness :
        public TextureAsset,
        public AssetAutoGuid<WoodenPier01PlanksRoughness> {
    public:
        WoodenPier01PlanksRoughness();
    };

    AUTO_REGISTER_ASSET(WoodenPier01PlanksRoughness)

}

#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class WoodenPier01PlanksRoughness :
        public TextureAsset,
        public AssetAutoGuid<WoodenPier01PlanksRoughness> {
    public:
        WoodenPier01PlanksRoughness();
    };
}

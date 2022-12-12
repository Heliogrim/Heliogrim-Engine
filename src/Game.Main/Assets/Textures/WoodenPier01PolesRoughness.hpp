#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class WoodenPier01PolesRoughness :
        public TextureAsset,
        public AssetAutoGuid<WoodenPier01PolesRoughness> {
    public:
        WoodenPier01PolesRoughness();
    };
}

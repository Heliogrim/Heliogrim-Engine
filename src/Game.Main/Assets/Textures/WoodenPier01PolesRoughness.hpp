#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class WoodenPier01PolesRoughness :
        public TextureAsset,
        public AssetAutoGuid<WoodenPier01PolesRoughness> {
    public:
        WoodenPier01PolesRoughness();
    };

    AUTO_REGISTER_ASSET(WoodenPier01PolesRoughness)

}

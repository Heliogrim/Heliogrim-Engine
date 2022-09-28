#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class WoodenPier01PolesNormal :
        public TextureAsset,
        public AssetAutoGuid<WoodenPier01PolesNormal> {
    public:
        WoodenPier01PolesNormal();
    };

    AUTO_REGISTER_ASSET(WoodenPier01PolesNormal)

}

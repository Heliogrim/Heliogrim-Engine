#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class WoodenPier01PlanksDiffuse :
        public TextureAsset,
        public AssetAutoGuid<WoodenPier01PlanksDiffuse> {
    public:
        WoodenPier01PlanksDiffuse();
    };

    AUTO_REGISTER_ASSET(WoodenPier01PlanksDiffuse)

}

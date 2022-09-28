#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class WoodenPier01PolesDiffuse :
        public TextureAsset,
        public AssetAutoGuid<WoodenPier01PolesDiffuse> {
    public:
        WoodenPier01PolesDiffuse();
    };

    AUTO_REGISTER_ASSET(WoodenPier01PolesDiffuse)

}

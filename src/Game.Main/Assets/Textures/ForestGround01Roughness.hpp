#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class ForestGround01Roughness :
        public TextureAsset,
        public AssetAutoGuid<ForestGround01Roughness> {
    public:
        ForestGround01Roughness();
    };

    AUTO_REGISTER_ASSET(ForestGround01Roughness)

}

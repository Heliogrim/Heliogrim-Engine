#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class ForestGround01Diffuse :
        public TextureAsset,
        public AssetAutoGuid<ForestGround01Diffuse> {
    public:
        ForestGround01Diffuse();
    };

    AUTO_REGISTER_ASSET(ForestGround01Diffuse)

}

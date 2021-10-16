#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::texture {

    class ForestGround01Albedo :
        public TextureAsset,
        public AssetAutoGuid<ForestGround01Albedo> {
    public:
        ForestGround01Albedo();
    };

    AUTO_REGISTER_ASSET(ForestGround01Albedo)

}

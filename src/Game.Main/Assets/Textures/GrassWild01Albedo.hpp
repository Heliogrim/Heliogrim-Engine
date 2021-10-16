#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/AssetAutoRegister.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {

    class GrassWild01Albedo :
        public TextureAsset,
        public AssetAutoGuid<GrassWild01Albedo> {
    public:
        GrassWild01Albedo();
    };

    AUTO_REGISTER_ASSET(GrassWild01Albedo)
}

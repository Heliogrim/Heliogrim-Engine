#pragma once
#include <Ember/AssetAutoGuid.hpp>

#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class GrassWild01Albedo :
        public TextureAsset,
        public AssetAutoGuid<GrassWild01Albedo> {
    public:
        GrassWild01Albedo();
    };
}

#pragma once
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class GrassWild01Specular :
        public TextureAsset,
        public AssetAutoGuid<GrassWild01Specular> {
    public:
        GrassWild01Specular();
    };
}

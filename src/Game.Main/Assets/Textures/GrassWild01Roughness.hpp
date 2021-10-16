#pragma once
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {

    class GrassWild01Roughness :
        public TextureAsset,
        public AssetAutoGuid<GrassWild01Roughness> {
    public:
        GrassWild01Roughness();
    };

}

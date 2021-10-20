#pragma once
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {

    class GrassWild01Cavity :
        public TextureAsset,
        public AssetAutoGuid<GrassWild01Cavity> {
    public:
        GrassWild01Cavity();
    };

    AUTO_REGISTER_ASSET(GrassWild01Cavity)
}

#pragma once
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {

    class GrassWild01Displacement :
        public TextureAsset,
        public AssetAutoGuid<GrassWild01Displacement> {
    public:
        GrassWild01Displacement();
    };

    AUTO_REGISTER_ASSET(GrassWild01Displacement)
}

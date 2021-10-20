#pragma once
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {

    class GrassWild01AO :
        public TextureAsset,
        public AssetAutoGuid<GrassWild01AO> {
    public:
        GrassWild01AO();
    };

    AUTO_REGISTER_ASSET(GrassWild01AO)
}

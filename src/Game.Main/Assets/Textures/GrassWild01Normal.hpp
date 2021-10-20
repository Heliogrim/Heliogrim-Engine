#pragma once
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {

    class GrassWild01Normal :
        public TextureAsset,
        public AssetAutoGuid<GrassWild01Normal> {
    public:
        GrassWild01Normal();
    };

    AUTO_REGISTER_ASSET(GrassWild01Normal)
}

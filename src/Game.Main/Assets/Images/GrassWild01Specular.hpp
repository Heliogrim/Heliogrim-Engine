#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class GrassWild01Specular :
        public ImageAsset,
        public AssetAutoGuid<GrassWild01Specular> {
    public:
        GrassWild01Specular() :
            ImageAsset(GrassWild01Specular::auto_guid()) {}
    };

    AUTO_REGISTER_ASSET(GrassWild01Specular)
}

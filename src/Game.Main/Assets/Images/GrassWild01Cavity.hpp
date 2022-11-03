#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class GrassWild01Cavity :
        public ImageAsset,
        public AssetAutoGuid<GrassWild01Cavity> {
    public:
        GrassWild01Cavity() :
            ImageAsset(GrassWild01Cavity::unstable_auto_guid()) {}
    };

    AUTO_REGISTER_ASSET(GrassWild01Cavity)
}

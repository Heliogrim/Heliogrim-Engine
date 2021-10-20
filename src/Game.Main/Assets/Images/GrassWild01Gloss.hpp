#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class GrassWild01Gloss :
        public ImageAsset,
        public AssetAutoGuid<GrassWild01Gloss> {
    public:
        GrassWild01Gloss() :
            ImageAsset(GrassWild01Gloss::auto_guid()) {}
    };

    AUTO_REGISTER_ASSET(GrassWild01Gloss)
}

#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class GrassWild01Normal :
        public ImageAsset,
        public AssetAutoGuid<GrassWild01Normal> {
    public:
        GrassWild01Normal() :
            ImageAsset(GrassWild01Normal::auto_guid()) {}
    };

    AUTO_REGISTER_ASSET(GrassWild01Normal)
}

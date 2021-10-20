#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class GrassWild01Displacement :
        public ImageAsset,
        public AssetAutoGuid<GrassWild01Displacement> {
    public:
        GrassWild01Displacement() :
            ImageAsset(GrassWild01Displacement::auto_guid()) {}
    };

    AUTO_REGISTER_ASSET(GrassWild01Displacement)
}

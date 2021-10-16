#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class GrassWild01Roughness :
        public ImageAsset,
        public AssetAutoGuid<GrassWild01Roughness> {
    public:
        GrassWild01Roughness() :
            ImageAsset(GrassWild01Roughness::auto_guid()) {}
    };

}

#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class GrassWild01Displacement :
        public ImageAsset,
        public AssetAutoGuid<GrassWild01Displacement> {
    public:
        GrassWild01Displacement() :
            ImageAsset(GrassWild01Displacement::unstable_auto_guid()) {}
    };
}

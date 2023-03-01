#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class GrassWild01Displacement :
        public ImageAsset,
        public AssetAutoGuid<GrassWild01Displacement> {
    public:
        GrassWild01Displacement() :
            ImageAsset(GrassWild01Displacement::unstable_auto_guid()) {}
    };
}

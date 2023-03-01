#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class GrassWild01Cavity :
        public ImageAsset,
        public AssetAutoGuid<GrassWild01Cavity> {
    public:
        GrassWild01Cavity() :
            ImageAsset(GrassWild01Cavity::unstable_auto_guid()) {}
    };
}

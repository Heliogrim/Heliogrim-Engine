#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class GrassWild01Normal :
        public ImageAsset,
        public AssetAutoGuid<GrassWild01Normal> {
    public:
        GrassWild01Normal() :
            ImageAsset(GrassWild01Normal::unstable_auto_guid()) {}
    };
}

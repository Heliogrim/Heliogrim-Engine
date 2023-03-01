#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class GrassWild01Roughness :
        public ImageAsset,
        public AssetAutoGuid<GrassWild01Roughness> {
    public:
        GrassWild01Roughness() :
            ImageAsset(GrassWild01Roughness::unstable_auto_guid()) {}
    };
}

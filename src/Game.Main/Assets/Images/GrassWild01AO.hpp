#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class GrassWild01AO :
        public ImageAsset,
        public AssetAutoGuid<GrassWild01AO> {
    public:
        GrassWild01AO() :
            ImageAsset(GrassWild01AO::unstable_auto_guid()) {}
    };
}

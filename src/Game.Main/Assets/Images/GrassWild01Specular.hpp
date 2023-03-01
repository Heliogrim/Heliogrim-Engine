#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class GrassWild01Specular :
        public ImageAsset,
        public AssetAutoGuid<GrassWild01Specular> {
    public:
        GrassWild01Specular() :
            ImageAsset(GrassWild01Specular::unstable_auto_guid()) {}
    };
}

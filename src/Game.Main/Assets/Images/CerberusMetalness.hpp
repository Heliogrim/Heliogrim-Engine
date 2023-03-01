#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class CerberusMetalness :
        public ImageAsset,
        public AssetAutoGuid<CerberusMetalness> {
    public:
        CerberusMetalness():
            ImageAsset(CerberusMetalness::unstable_auto_guid(),
                R"(resources\imports\ktx2\cerberus_metalness.ktx2)") {}
    };
}

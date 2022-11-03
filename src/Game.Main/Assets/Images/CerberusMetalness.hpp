#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class CerberusMetalness:
        public ImageAsset,
        public AssetAutoGuid<CerberusMetalness> {
    public:
        CerberusMetalness():
            ImageAsset(CerberusMetalness::unstable_auto_guid(),
                R"(resources\imports\ktx2\cerberus_metalness.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(CerberusMetalness)
}

#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class CerberusMetalness:
        public ImageAsset,
        public AssetAutoGuid<CerberusMetalness> {
    public:
        CerberusMetalness():
            ImageAsset(CerberusMetalness::auto_guid(),
                R"(resources\assets\texture\cerberus_metalness.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(CerberusMetalness)
}

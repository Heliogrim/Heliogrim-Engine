#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class CerberusRoughness:
        public ImageAsset,
        public AssetAutoGuid<CerberusRoughness> {
    public:
        CerberusRoughness():
            ImageAsset(CerberusRoughness::auto_guid(),
                R"(resources\assets\texture\cerberus_roughness.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(CerberusRoughness)
}

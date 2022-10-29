#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class CerberusDiffuse:
        public ImageAsset,
        public AssetAutoGuid<CerberusDiffuse> {
    public:
        CerberusDiffuse():
            ImageAsset(CerberusDiffuse::auto_guid(),
                R"(resources\imports\ktx2\cerberus_diffuse.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(CerberusDiffuse)
}

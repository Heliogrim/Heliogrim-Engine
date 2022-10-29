#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class CerberusNormal:
        public ImageAsset,
        public AssetAutoGuid<CerberusNormal> {
    public:
        CerberusNormal():
            ImageAsset(CerberusNormal::auto_guid(),
                R"(resources\imports\ktx2\cerberus_normal.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(CerberusNormal)
}

#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class CerberusAO:
        public ImageAsset,
        public AssetAutoGuid<CerberusAO> {
    public:
        CerberusAO():
            ImageAsset(CerberusAO::auto_guid(),
                R"(resources\assets\texture\cerberus_ao.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(CerberusAO)
}

#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class CerberusRoughness :
        public ImageAsset,
        public AssetAutoGuid<CerberusRoughness> {
    public:
        CerberusRoughness():
            ImageAsset(CerberusRoughness::unstable_auto_guid(),
                R"(resources\imports\ktx2\cerberus_roughness.ktx2)") {}
    };
}

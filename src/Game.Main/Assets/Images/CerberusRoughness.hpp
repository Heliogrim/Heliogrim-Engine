#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class CerberusRoughness :
        public ImageAsset,
        public AssetAutoGuid<CerberusRoughness> {
    public:
        CerberusRoughness():
            ImageAsset(CerberusRoughness::unstable_auto_guid(),
                R"(resources\imports\ktx2\cerberus_roughness.ktx2)") {}
    };
}

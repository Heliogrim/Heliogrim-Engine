#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class CerberusDiffuse :
        public ImageAsset,
        public AssetAutoGuid<CerberusDiffuse> {
    public:
        CerberusDiffuse():
            ImageAsset(CerberusDiffuse::unstable_auto_guid(),
                R"(resources\imports\ktx2\cerberus_diffuse.ktx2)") {}
    };
}

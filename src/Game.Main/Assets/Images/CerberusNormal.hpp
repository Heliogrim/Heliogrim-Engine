#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class CerberusNormal :
        public ImageAsset,
        public AssetAutoGuid<CerberusNormal> {
    public:
        CerberusNormal():
            ImageAsset(CerberusNormal::unstable_auto_guid(),
                R"(resources\imports\ktx2\cerberus_normal.ktx2)") {}
    };
}

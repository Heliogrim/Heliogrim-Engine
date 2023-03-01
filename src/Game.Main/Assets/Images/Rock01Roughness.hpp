#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class Rock01Roughness :
        public ImageAsset,
        public AssetAutoGuid<Rock01Roughness> {
    public:
        Rock01Roughness():
            ImageAsset(Rock01Roughness::unstable_auto_guid(),
                R"(resources\imports\ktx2\rock_01_roughness.ktx2)") {}
    };
}

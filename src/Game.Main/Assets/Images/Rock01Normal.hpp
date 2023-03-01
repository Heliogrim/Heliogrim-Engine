#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class Rock01Normal :
        public ImageAsset,
        public AssetAutoGuid<Rock01Normal> {
    public:
        Rock01Normal():
            ImageAsset(Rock01Normal::unstable_auto_guid(),
                R"(resources\imports\ktx2\rock_01_normal.ktx2)") {}
    };
}

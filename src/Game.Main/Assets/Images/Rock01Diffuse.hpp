#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class Rock01Diffuse :
        public ImageAsset,
        public AssetAutoGuid<Rock01Diffuse> {
    public:
        Rock01Diffuse():
            ImageAsset(Rock01Diffuse::unstable_auto_guid(),
                R"(resources\imports\ktx2\rock_01_diffuse.ktx2)") {}
    };
}

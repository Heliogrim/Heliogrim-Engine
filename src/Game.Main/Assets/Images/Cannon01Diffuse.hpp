#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class Cannon01Diffuse :
        public ImageAsset,
        public AssetAutoGuid<Cannon01Diffuse> {
    public:
        Cannon01Diffuse():
            ImageAsset(Cannon01Diffuse::unstable_auto_guid(),
                R"(resources\imports\ktx2\cannon_01_diffuse.ktx2)") {}
    };
}

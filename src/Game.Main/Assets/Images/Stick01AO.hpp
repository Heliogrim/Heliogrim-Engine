#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class Stick01AO :
        public ImageAsset,
        public AssetAutoGuid<Stick01AO> {
    public:
        Stick01AO():
            ImageAsset(Stick01AO::unstable_auto_guid(),
                R"(resources\imports\ktx2\stick_01_ao.ktx2)") {}
    };
}

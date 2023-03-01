#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class Stick01Normal :
        public ImageAsset,
        public AssetAutoGuid<Stick01Normal> {
    public:
        Stick01Normal():
            ImageAsset(Stick01Normal::unstable_auto_guid(),
                R"(resources\imports\ktx2\stick_01_normal.ktx2)") {}
    };
}

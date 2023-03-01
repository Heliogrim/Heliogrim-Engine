#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class Icon3d :
        public ImageAsset,
        public AssetAutoGuid<Icon3d> {
    public:
        Icon3d():
            ImageAsset(
                Icon3d::unstable_auto_guid(),
                R"(resources\imports\ktx\3d.ktx)"
            ) {}
    };
}

#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconImage :
        public ImageAsset,
        public AssetAutoGuid<IconImage> {
    public:
        IconImage():
            ImageAsset(
                IconImage::unstable_auto_guid(),
                R"(resources\imports\ktx\image.ktx)"
            ) {}
    };
}

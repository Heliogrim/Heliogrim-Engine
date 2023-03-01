#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconVideo :
        public ImageAsset,
        public AssetAutoGuid<IconVideo> {
    public:
        IconVideo():
            ImageAsset(
                IconVideo::unstable_auto_guid(),
                R"(resources\imports\ktx\video.ktx)"
            ) {}
    };
}

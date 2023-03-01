#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconUrl :
        public ImageAsset,
        public AssetAutoGuid<IconUrl> {
    public:
        IconUrl():
            ImageAsset(
                IconUrl::unstable_auto_guid(),
                R"(resources\imports\ktx\url.ktx)"
            ) {}
    };
}

#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconFavicon :
        public ImageAsset,
        public AssetAutoGuid<IconFavicon> {
    public:
        IconFavicon():
            ImageAsset(
                IconFavicon::unstable_auto_guid(),
                R"(resources\imports\ktx\favicon.ktx)"
            ) {}
    };
}

#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconFont :
        public ImageAsset,
        public AssetAutoGuid<IconFont> {
    public:
        IconFont():
            ImageAsset(
                IconFont::unstable_auto_guid(),
                R"(resources\imports\ktx\font.ktx)"
            ) {}
    };
}

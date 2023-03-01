#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconReadme :
        public ImageAsset,
        public AssetAutoGuid<IconReadme> {
    public:
        IconReadme():
            ImageAsset(
                IconReadme::unstable_auto_guid(),
                R"(resources\imports\ktx\readme.ktx)"
            ) {}
    };
}

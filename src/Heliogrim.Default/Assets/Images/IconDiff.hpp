#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconDiff :
        public ImageAsset,
        public AssetAutoGuid<IconDiff> {
    public:
        IconDiff():
            ImageAsset(
                IconDiff::unstable_auto_guid(),
                R"(resources\imports\ktx\diff.ktx)"
            ) {}
    };
}

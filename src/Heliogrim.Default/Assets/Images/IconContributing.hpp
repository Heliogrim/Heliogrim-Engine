#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconContributing :
        public ImageAsset,
        public AssetAutoGuid<IconContributing> {
    public:
        IconContributing():
            ImageAsset(
                IconContributing::unstable_auto_guid(),
                R"(resources\imports\ktx\contributing.ktx)"
            ) {}
    };
}

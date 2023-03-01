#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconSearch :
        public ImageAsset,
        public AssetAutoGuid<IconSearch> {
    public:
        IconSearch():
            ImageAsset(
                IconSearch::unstable_auto_guid(),
                R"(resources\imports\ktx\search.ktx)"
            ) {}
    };
}

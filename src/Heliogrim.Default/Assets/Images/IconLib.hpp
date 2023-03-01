#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconLib :
        public ImageAsset,
        public AssetAutoGuid<IconLib> {
    public:
        IconLib():
            ImageAsset(
                IconLib::unstable_auto_guid(),
                R"(resources\imports\ktx\lib.ktx)"
            ) {}
    };
}

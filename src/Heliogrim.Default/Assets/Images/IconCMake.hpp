#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconCMake :
        public ImageAsset,
        public AssetAutoGuid<IconCMake> {
    public:
        IconCMake():
            ImageAsset(
                IconCMake::unstable_auto_guid(),
                R"(resources\imports\ktx\cmake.ktx)"
            ) {}
    };
}

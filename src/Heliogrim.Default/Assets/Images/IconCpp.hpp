#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconCpp :
        public ImageAsset,
        public AssetAutoGuid<IconCpp> {
    public:
        IconCpp():
            ImageAsset(
                IconCpp::unstable_auto_guid(),
                R"(resources\imports\ktx\cpp.ktx)"
            ) {}
    };
}

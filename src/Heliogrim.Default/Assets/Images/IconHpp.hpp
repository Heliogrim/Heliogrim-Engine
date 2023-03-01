#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconHpp :
        public ImageAsset,
        public AssetAutoGuid<IconHpp> {
    public:
        IconHpp():
            ImageAsset(
                IconHpp::unstable_auto_guid(),
                R"(resources\imports\ktx\hpp.ktx)"
            ) {}
    };
}

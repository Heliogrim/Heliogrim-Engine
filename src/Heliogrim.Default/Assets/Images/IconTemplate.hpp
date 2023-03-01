#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconTemplate :
        public ImageAsset,
        public AssetAutoGuid<IconTemplate> {
    public:
        IconTemplate():
            ImageAsset(
                IconTemplate::unstable_auto_guid(),
                R"(resources\imports\ktx\template.ktx)"
            ) {}
    };
}

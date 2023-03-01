#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconSettings :
        public ImageAsset,
        public AssetAutoGuid<IconSettings> {
    public:
        IconSettings():
            ImageAsset(
                IconSettings::unstable_auto_guid(),
                R"(resources\imports\ktx\settings.ktx)"
            ) {}
    };
}

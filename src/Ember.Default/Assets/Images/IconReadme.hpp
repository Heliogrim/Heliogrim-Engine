#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconReadme :
        public ImageAsset,
        public AssetAutoGuid<IconReadme> {
    public:
        IconReadme():
            ImageAsset(IconReadme::auto_guid(),
                R"(resources\imports\ktx\readme.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconReadme)
}

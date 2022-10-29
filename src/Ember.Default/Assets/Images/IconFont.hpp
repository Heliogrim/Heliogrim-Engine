#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconFont :
        public ImageAsset,
        public AssetAutoGuid<IconFont> {
    public:
        IconFont():
            ImageAsset(IconFont::auto_guid(),
                R"(resources\imports\ktx\font.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconFont)
}

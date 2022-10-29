#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconFavicon :
        public ImageAsset,
        public AssetAutoGuid<IconFavicon> {
    public:
        IconFavicon():
            ImageAsset(IconFavicon::auto_guid(),
                R"(resources\imports\ktx\favicon.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconFavicon)
}

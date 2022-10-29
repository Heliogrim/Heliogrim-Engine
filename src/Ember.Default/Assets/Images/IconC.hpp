#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconC :
        public ImageAsset,
        public AssetAutoGuid<IconC> {
    public:
        IconC():
            ImageAsset(IconC::auto_guid(),
                R"(resources\imports\ktx\c.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconC)
}

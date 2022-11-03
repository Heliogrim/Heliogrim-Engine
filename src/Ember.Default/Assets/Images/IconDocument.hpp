#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconDocument :
        public ImageAsset,
        public AssetAutoGuid<IconDocument> {
    public:
        IconDocument():
            ImageAsset(IconDocument::unstable_auto_guid(),
                R"(resources\imports\ktx\document.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconDocument)
}

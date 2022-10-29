#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconArchitecture :
        public ImageAsset,
        public AssetAutoGuid<IconArchitecture> {
    public:
        IconArchitecture():
            ImageAsset(IconArchitecture::auto_guid(),
                R"(resources\imports\ktx\architecture.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconArchitecture)
}

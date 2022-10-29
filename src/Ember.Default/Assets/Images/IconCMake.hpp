#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconCMake :
        public ImageAsset,
        public AssetAutoGuid<IconCMake> {
    public:
        IconCMake():
            ImageAsset(IconCMake::auto_guid(),
                R"(resources\imports\ktx\cmake.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconCMake)
}

#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconSettings :
        public ImageAsset,
        public AssetAutoGuid<IconSettings> {
    public:
        IconSettings():
            ImageAsset(IconSettings::auto_guid(),
                R"(resources\imports\ktx\settings.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconSettings)
}

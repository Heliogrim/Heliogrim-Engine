#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconZip :
        public ImageAsset,
        public AssetAutoGuid<IconZip> {
    public:
        IconZip():
            ImageAsset(IconZip::unstable_auto_guid(),
                R"(resources\imports\ktx\zip.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconZip)
}

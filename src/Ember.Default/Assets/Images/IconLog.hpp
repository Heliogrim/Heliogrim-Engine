#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconLog :
        public ImageAsset,
        public AssetAutoGuid<IconLog> {
    public:
        IconLog():
            ImageAsset(IconLog::auto_guid(),
                R"(resources\imports\ktx\log.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconLog)
}

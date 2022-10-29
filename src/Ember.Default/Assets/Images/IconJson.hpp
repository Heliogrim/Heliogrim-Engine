#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconJson :
        public ImageAsset,
        public AssetAutoGuid<IconJson> {
    public:
        IconJson():
            ImageAsset(IconJson::auto_guid(),
                R"(resources\imports\ktx\json.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconJson)
}

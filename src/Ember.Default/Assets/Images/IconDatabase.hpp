#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconDatabase :
        public ImageAsset,
        public AssetAutoGuid<IconDatabase> {
    public:
        IconDatabase():
            ImageAsset(IconDatabase::auto_guid(),
                R"(resources\imports\ktx\database.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconDatabase)
}

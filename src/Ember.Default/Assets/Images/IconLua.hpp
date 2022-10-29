#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconLua :
        public ImageAsset,
        public AssetAutoGuid<IconLua> {
    public:
        IconLua():
            ImageAsset(IconLua::auto_guid(),
                R"(resources\imports\ktx\lua.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconLua)
}

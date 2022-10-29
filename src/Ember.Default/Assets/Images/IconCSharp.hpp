#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconCSharp :
        public ImageAsset,
        public AssetAutoGuid<IconCSharp> {
    public:
        IconCSharp():
            ImageAsset(IconCSharp::auto_guid(),
                R"(resources\imports\ktx\csharp.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconCSharp)
}



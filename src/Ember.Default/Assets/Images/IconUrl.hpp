﻿#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconUrl :
        public ImageAsset,
        public AssetAutoGuid<IconUrl> {
    public:
        IconUrl():
            ImageAsset(IconUrl::auto_guid(),
                R"(resources\imports\ktx\url.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconUrl)
}

﻿#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconVideo :
        public ImageAsset,
        public AssetAutoGuid<IconVideo> {
    public:
        IconVideo():
            ImageAsset(IconVideo::auto_guid(),
                R"(resources\imports\ktx\video.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconVideo)
}

﻿#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconImage :
        public ImageAsset,
        public AssetAutoGuid<IconImage> {
    public:
        IconImage():
            ImageAsset(IconImage::unstable_auto_guid(),
                R"(resources\imports\ktx\image.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconImage)
}

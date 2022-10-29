﻿#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconI18n :
        public ImageAsset,
        public AssetAutoGuid<IconI18n> {
    public:
        IconI18n():
            ImageAsset(IconI18n::auto_guid(),
                R"(resources\imports\ktx\i18n.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconI18n)
}

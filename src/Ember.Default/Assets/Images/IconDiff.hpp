﻿#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class IconDiff :
        public ImageAsset,
        public AssetAutoGuid<IconDiff> {
    public:
        IconDiff():
            ImageAsset(IconDiff::unstable_auto_guid(),
                R"(resources\imports\ktx\diff.ktx)") {}
    };
}

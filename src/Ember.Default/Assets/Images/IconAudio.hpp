﻿#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconAudio :
        public ImageAsset,
        public AssetAutoGuid<IconAudio> {
    public:
        IconAudio():
            ImageAsset(IconAudio::auto_guid(),
                R"(resources\imports\ktx\audio.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconAudio)
}

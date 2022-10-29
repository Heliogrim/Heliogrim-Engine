﻿#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class FolderAnimation :
        public ImageAsset,
        public AssetAutoGuid<FolderAnimation> {
    public:
        FolderAnimation():
            ImageAsset(FolderAnimation::auto_guid(),
                R"(resources\imports\ktx\folder-animation.ktx)") {}
    };

    AUTO_REGISTER_ASSET(FolderAnimation)
}

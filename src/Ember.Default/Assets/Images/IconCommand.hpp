﻿#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconCommand :
        public ImageAsset,
        public AssetAutoGuid<IconCommand> {
    public:
        IconCommand():
            ImageAsset(IconCommand::auto_guid(),
                R"(resources\imports\ktx\command.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconCommand)
}

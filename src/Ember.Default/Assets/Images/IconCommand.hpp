#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class IconCommand :
        public ImageAsset,
        public AssetAutoGuid<IconCommand> {
    public:
        IconCommand():
            ImageAsset(
                IconCommand::unstable_auto_guid(),
                R"(resources\imports\ktx\command.ktx)"
            ) {}
    };
}

#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
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

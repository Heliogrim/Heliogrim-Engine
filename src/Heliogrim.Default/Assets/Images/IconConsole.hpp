#pragma once
#include <Heliogrim/ImageAsset.hpp>

namespace hg::game::assets::image {
    class IconConsole :
        public ImageAsset,
        public AssetAutoGuid<IconConsole> {
    public:
        IconConsole():
            ImageAsset(
                IconConsole::unstable_auto_guid(),
                R"(resources\imports\ktx\console.ktx)"
            ) {}
    };
}

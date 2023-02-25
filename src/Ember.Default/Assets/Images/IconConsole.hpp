#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
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

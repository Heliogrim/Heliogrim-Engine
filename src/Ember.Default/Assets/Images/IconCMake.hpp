#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class IconCMake :
        public ImageAsset,
        public AssetAutoGuid<IconCMake> {
    public:
        IconCMake():
            ImageAsset(
                IconCMake::unstable_auto_guid(),
                R"(resources\imports\ktx\cmake.ktx)"
            ) {}
    };
}

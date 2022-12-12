#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class IconLib :
        public ImageAsset,
        public AssetAutoGuid<IconLib> {
    public:
        IconLib():
            ImageAsset(IconLib::unstable_auto_guid(),
                R"(resources\imports\ktx\lib.ktx)") {}
    };
}

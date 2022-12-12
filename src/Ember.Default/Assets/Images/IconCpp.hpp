#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class IconCpp :
        public ImageAsset,
        public AssetAutoGuid<IconCpp> {
    public:
        IconCpp():
            ImageAsset(IconCpp::unstable_auto_guid(),
                R"(resources\imports\ktx\cpp.ktx)") {}
    };
}

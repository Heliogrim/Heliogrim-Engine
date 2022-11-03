#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconHpp :
        public ImageAsset,
        public AssetAutoGuid<IconHpp> {
    public:
        IconHpp():
            ImageAsset(IconHpp::unstable_auto_guid(),
                R"(resources\imports\ktx\hpp.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconHpp)
}

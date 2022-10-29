#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class IconH :
        public ImageAsset,
        public AssetAutoGuid<IconH> {
    public:
        IconH():
            ImageAsset(IconH::auto_guid(),
                R"(resources\imports\ktx\h.ktx)") {}
    };

    AUTO_REGISTER_ASSET(IconH)
}

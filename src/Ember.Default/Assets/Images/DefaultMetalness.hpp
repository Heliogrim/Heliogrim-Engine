#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class DefaultMetalness :
        public ImageAsset,
        public AssetAutoGuid<DefaultMetalness> {
    public:
        DefaultMetalness():
            ImageAsset(DefaultMetalness::unstable_auto_guid(),
                R"(resources\imports\ktx\default_metalness.ktx)") {}
    };

    AUTO_REGISTER_ASSET(DefaultMetalness)
}

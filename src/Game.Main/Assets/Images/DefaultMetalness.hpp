#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class DefaultMetalness :
        public ImageAsset,
        public AssetAutoGuid<DefaultMetalness> {
    public:
        DefaultMetalness():
            ImageAsset(DefaultMetalness::auto_guid(),
                R"(resources\assets\texture\default_metalness.ktx)") {}
    };

    AUTO_REGISTER_ASSET(DefaultMetalness)
}

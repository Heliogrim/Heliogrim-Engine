#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class DefaultRoughness :
        public ImageAsset,
        public AssetAutoGuid<DefaultRoughness> {
    public:
        DefaultRoughness():
            ImageAsset(DefaultRoughness::auto_guid(),
                R"(resources\assets\texture\default_roughness.ktx)") {}
    };

    AUTO_REGISTER_ASSET(DefaultRoughness)
}

#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class DefaultAO :
        public ImageAsset,
        public AssetAutoGuid<DefaultAO> {
    public:
        DefaultAO():
            ImageAsset(DefaultAO::auto_guid(),
                R"(resources\assets\texture\default_ao.ktx)") {}
    };

    AUTO_REGISTER_ASSET(DefaultAO)
}

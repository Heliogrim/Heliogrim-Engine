#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class DefaultNormal :
        public ImageAsset,
        public AssetAutoGuid<DefaultNormal> {
    public:
        DefaultNormal():
            ImageAsset(DefaultNormal::auto_guid(),
                R"(resources\assets\texture\default_normal.ktx)") {}
    };

    AUTO_REGISTER_ASSET(DefaultNormal)
}

#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class DefaultDiffuse :
        public ImageAsset,
        public AssetAutoGuid<DefaultDiffuse> {
    public:
        DefaultDiffuse():
            ImageAsset(DefaultDiffuse::auto_guid(),
                R"(resources\assets\texture\default_diffuse.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(DefaultDiffuse)
}

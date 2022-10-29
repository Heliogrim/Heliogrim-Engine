#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class DefaultSkybox :
        public ImageAsset,
        public AssetAutoGuid<DefaultSkybox> {
    public:
        DefaultSkybox():
            ImageAsset(DefaultSkybox::auto_guid(),
                R"(resources\imports\ktx\default_skybox.ktx)") {}
    };

    AUTO_REGISTER_ASSET(DefaultSkybox)
}

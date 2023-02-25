#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {
    class DefaultSkybox :
        public ImageAsset,
        public AssetAutoGuid<DefaultSkybox> {
    public:
        DefaultSkybox():
            ImageAsset(
                DefaultSkybox::unstable_auto_guid(),
                R"(resources\imports\ktx\default_skybox.ktx)"
            ) {}
    };
}

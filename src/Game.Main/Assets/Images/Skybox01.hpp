#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class Skybox01Diffuse :
        public ImageAsset,
        public AssetAutoGuid<Skybox01Diffuse> {
    public:
        Skybox01Diffuse():
            ImageAsset(Skybox01Diffuse::unstable_auto_guid(),
                R"(resources\imports\ktx\skybox_01_diffuse.ktx)") {}
    };

    AUTO_REGISTER_ASSET(Skybox01Diffuse)
}

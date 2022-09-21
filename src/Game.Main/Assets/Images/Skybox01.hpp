#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class Skybox01Diffuse :
        public ImageAsset,
        public AssetAutoGuid<Skybox01Diffuse> {
    public:
        Skybox01Diffuse():
            ImageAsset(Skybox01Diffuse::auto_guid(),
                R"(resources\assets\texture\skybox_01_diffuse.ktx)") {}
    };

    AUTO_REGISTER_ASSET(Skybox01Diffuse)
}

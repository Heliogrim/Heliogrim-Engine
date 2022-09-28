#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class Rock01Diffuse :
        public ImageAsset,
        public AssetAutoGuid<Rock01Diffuse> {
    public:
        Rock01Diffuse():
            ImageAsset(Rock01Diffuse::auto_guid(),
                R"(resources\assets\texture\rock_01_diffuse.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(Rock01Diffuse)
}

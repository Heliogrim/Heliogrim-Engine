#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class Rock01Roughness :
        public ImageAsset,
        public AssetAutoGuid<Rock01Roughness> {
    public:
        Rock01Roughness():
            ImageAsset(Rock01Roughness::auto_guid(),
                R"(resources\imports\ktx2\rock_01_roughness.ktx2)") {}
    };

    AUTO_REGISTER_ASSET(Rock01Roughness)
}

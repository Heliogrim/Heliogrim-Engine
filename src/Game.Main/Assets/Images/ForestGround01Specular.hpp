#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class ForestGround01Specular :
        public ImageAsset,
        public AssetAutoGuid<ForestGround01Specular> {
    public:
        ForestGround01Specular() :
            ImageAsset(ForestGround01Specular::auto_guid()) {}
    };

    AUTO_REGISTER_ASSET(ForestGround01Specular)

}

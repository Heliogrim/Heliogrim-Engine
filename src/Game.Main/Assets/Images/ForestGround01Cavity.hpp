#pragma once
#include <Ember/ImageAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::image {

    class ForestGround01Cavity :
        public ImageAsset,
        public AssetAutoGuid<ForestGround01Cavity> {
    public:
        ForestGround01Cavity() :
            ImageAsset(ForestGround01Cavity::auto_guid()) {}
    };

    AUTO_REGISTER_ASSET(ForestGround01Cavity)

}

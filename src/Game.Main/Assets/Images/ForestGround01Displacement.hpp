#pragma once
#include <Ember/ImageAsset.hpp>
#include <Ember/AssetAutoRegister.hpp>

namespace ember::game::assets::image {

    class ForestGround01Displacement :
        public ImageAsset,
        public AssetAutoGuid<ForestGround01Displacement> {
    public:
        ForestGround01Displacement() :
            ImageAsset(ForestGround01Displacement::auto_guid()) {}
    };

    AUTO_REGISTER_ASSET(ForestGround01Displacement)

}

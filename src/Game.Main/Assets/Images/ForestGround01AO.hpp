#pragma once
#include <Ember/AssetAutoRegister.hpp>
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class ForestGround01AO :
        public ImageAsset,
        public AssetAutoGuid<ForestGround01AO> {
    public:
        ForestGround01AO() :
            ImageAsset(ForestGround01AO::auto_guid()) {}
    };

    AUTO_REGISTER_ASSET(ForestGround01AO)

}

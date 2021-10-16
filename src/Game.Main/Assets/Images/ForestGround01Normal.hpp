#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class ForestGround01Normal :
        public ImageAsset,
        public AssetAutoGuid<ForestGround01Normal> {
    public:
        ForestGround01Normal() :
            ImageAsset(ForestGround01Normal::auto_guid()) {}
    };

    AUTO_REGISTER_ASSET(ForestGround01Normal)
}

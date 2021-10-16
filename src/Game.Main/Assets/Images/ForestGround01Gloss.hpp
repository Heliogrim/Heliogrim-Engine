#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class ForestGround01Gloss :
        public ImageAsset,
        public AssetAutoGuid<ForestGround01Gloss> {
    public:
        ForestGround01Gloss() :
            ImageAsset(ForestGround01Gloss::auto_guid()) {}
    };

    AUTO_REGISTER_ASSET(ForestGround01Gloss)
}

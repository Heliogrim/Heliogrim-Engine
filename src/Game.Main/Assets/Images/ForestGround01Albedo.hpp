#pragma once
#include <Ember/ImageAsset.hpp>

namespace ember::game::assets::image {

    class ForestGround01Albedo :
        public ImageAsset,
        public AssetAutoGuid<ForestGround01Albedo> {
    public:
        ForestGround01Albedo() :
            ImageAsset(ForestGround01Albedo::auto_guid()) {}
    };

    AUTO_REGISTER_ASSET(ForestGround01Albedo)
}

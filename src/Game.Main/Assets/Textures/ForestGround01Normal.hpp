#pragma once
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {

    class ForestGround01Normal :
        public TextureAsset,
        public AssetAutoGuid<ForestGround01Normal> {
    public:
        ForestGround01Normal();
    };

    AUTO_REGISTER_ASSET(ForestGround01Normal)
}

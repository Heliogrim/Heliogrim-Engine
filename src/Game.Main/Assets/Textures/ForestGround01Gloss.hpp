#pragma once
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {

    class ForestGround01Gloss :
        public TextureAsset,
        public AssetAutoGuid<ForestGround01Gloss> {
    public:
        ForestGround01Gloss();
    };

    AUTO_REGISTER_ASSET(ForestGround01Gloss)

}

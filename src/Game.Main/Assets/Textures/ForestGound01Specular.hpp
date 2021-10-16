#pragma once
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {

    class ForestGround01Specular :
        public TextureAsset,
        public AssetAutoGuid<ForestGround01Specular> {
    public:
        ForestGround01Specular();
    };

}

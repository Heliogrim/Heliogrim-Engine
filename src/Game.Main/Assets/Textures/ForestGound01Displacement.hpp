#pragma once
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {

    class ForestGround01Displacement :
        public TextureAsset,
        public AssetAutoGuid<ForestGround01Displacement> {
    public:
        ForestGround01Displacement();
    };

}

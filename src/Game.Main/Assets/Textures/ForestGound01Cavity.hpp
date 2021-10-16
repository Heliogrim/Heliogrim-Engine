#pragma once
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {

    class ForestGround01Cavity :
        public TextureAsset,
        public AssetAutoGuid<ForestGround01Cavity> {
    public:
        ForestGround01Cavity();
    };

}

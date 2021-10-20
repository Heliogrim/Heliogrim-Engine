#pragma once
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {

    class ForestGround01AO :
        public TextureAsset,
        public AssetAutoGuid<ForestGround01AO> {
    public:
        ForestGround01AO();
    };

    AUTO_REGISTER_ASSET(ForestGround01AO)
}

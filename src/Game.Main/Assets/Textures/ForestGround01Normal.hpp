#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class ForestGround01Normal :
        public TextureAsset,
        public AssetAutoGuid<ForestGround01Normal> {
    public:
        ForestGround01Normal();
    };
}

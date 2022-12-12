#pragma once
#include <Ember/AssetAutoGuid.hpp>
#include <Ember/TextureAsset.hpp>

namespace ember::game::assets::texture {
    class WoodenPier01PolesDiffuse :
        public TextureAsset,
        public AssetAutoGuid<WoodenPier01PolesDiffuse> {
    public:
        WoodenPier01PolesDiffuse();
    };
}
